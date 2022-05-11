#include "parser.h"
#include <tuple>

File::File() {
    statements = std::vector<Statement*>();
}

Parser::Parser(std::vector<Token>& tokens) {
    this->tokens = tokens;
    this->current = 0;
    this->root = File();
}

void Parser::parse() {
    while (current < tokens.size()) {
        root.statements.push_back(eval_statement());
    }

    auto import_stmts = std::vector<ImportStatement*>();
    auto other_stmts = std::vector<Statement*>();
    for (int i = 0; i < root.statements.size(); i++) {
        auto ptr = dynamic_cast<ImportStatement*>(root.statements.at(i));
        if (ptr) {
            import_stmts.push_back(ptr);
        }
        else {
            other_stmts.push_back(root.statements.at(i));
        }
    }

    root.statements = other_stmts;

    for (auto i : import_stmts) {
        auto string_lit = dynamic_cast<StringLiteralExpression*>(i->file);
        if (!string_lit) { continue; }

        auto lexer = Lexer();
        lexer.lex(string_lit->token.string.c_str());

        auto other_parser = Parser(lexer.tokens);
        other_parser.parse();

        for (auto stmt : root.statements) {
            other_parser.root.statements.push_back(stmt);
        }

        root = other_parser.root;
    }
}

Statement* Parser::eval_statement() {
    switch (peek()->type)
    {
    case TOKEN_LET:
        return eval_let_statement();
        break;
    case TOKEN_FN:
        return eval_fn_statement();
        break;
    case TOKEN_LOOP:
        return eval_loop_statement();
        break;
    case TOKEN_STRUCT:
        return eval_struct_statement();
        break;
    case TOKEN_INSERT:
        return eval_insert_statement();
        break;
    case TOKEN_RETURN:
        return eval_return_statement();
        break;
    case TOKEN_BREAK:
        return eval_break_statement();
        break;
    case TOKEN_IMPORT:
        return eval_import_statement();
        break;
    case TOKEN_IDENTIFIER:
        // x := y; ..or.. x();
        if (peek(1)->type == TOKEN_EQUAL) {
            return eval_assigment_statement();
        }

        return eval_expression_statement();
        break;
    default:
        panic("Cannot parse token as the begining of a statement");
        break;
    }

    if (peek()->type == TOKEN_LET)
        return eval_let_statement();
    else
        return eval_insert_statement();
}

LetStatement* Parser::eval_let_statement() {
    consume_token_of_type(TOKEN_LET);
    Token* identifier = consume_token_of_type(TOKEN_IDENTIFIER);

    // might be walrus or explicit type
    if(peek(0)->type == TOKEN_COLON) {
        consume_token_of_type(TOKEN_COLON);
        auto type = eval_type_expression();
        consume_token_of_type(TOKEN_EQUAL);
        auto expression = eval_expression_statement()->expression;
        return new LetStatement(*identifier, expression, type);
    } else {
        consume_token_of_type(TOKEN_WALRUS);
        auto expression = eval_expression_statement()->expression;
        return new LetStatement(*identifier, expression, nullptr);
    }
}

ScopeStatement* Parser::eval_scope_statement() {
    auto statements = std::vector<Statement*>();
    consume_token_of_type(TOKEN_BRACE_OPEN);
    int closing_brace_index = find_balance_point(TOKEN_BRACE_OPEN, TOKEN_BRACE_CLOSE, current - 1);
    if (closing_brace_index == current + 1) { // if this scope is empty
        current++;
    }
    while (current < closing_brace_index) {
        statements.push_back(eval_statement());
    }
    consume_token_of_type(TOKEN_BRACE_CLOSE);

    return new ScopeStatement(statements);
}

FnStatement* Parser::eval_fn_statement() {
    consume_token_of_type(TOKEN_FN);
    Token* identifier = consume_token_of_type(TOKEN_IDENTIFIER);
    consume_token_of_type(TOKEN_PAREN_OPEN);
    auto params = consume_comma_seperated_values();
    consume_token_of_type(TOKEN_PAREN_CLOSE);
    consume_token_of_type(TOKEN_COLON);
    auto type = eval_type_expression();

    auto body = eval_scope_statement();

    return new FnStatement(*identifier, params, type, body);
}

LoopStatement* Parser::eval_loop_statement() {
    consume_token_of_type(TOKEN_LOOP);
    Token* identifier = consume_token_of_type(TOKEN_STRING_LITERAL);
    auto body = eval_scope_statement();

    return new LoopStatement(*identifier, body);
}

int Parser::find_balance_point(TokenType push, TokenType pull, int from) {
    int current_index = from;
    int balance = 0;

    while (current_index < tokens.size()) {
        if (tokens.at(current_index).type == push) {
            balance++;
            if (balance == 0)
                return current_index;
        }
        if (tokens.at(current_index).type == pull) {
            balance--;
            if (balance == 0)
                return current_index;
        }

        current_index++;
    }

    return -1;
}

StructStatement* Parser::eval_struct_statement() {
    consume_token_of_type(TOKEN_STRUCT);
    Token* identifier = consume_token_of_type(TOKEN_IDENTIFIER);
    consume_token_of_type(TOKEN_BRACE_OPEN);
    auto member = consume_comma_seperated_values(); 
    consume_token_of_type(TOKEN_BRACE_CLOSE);

    return new StructStatement(*identifier, member);
}

InsertStatement* Parser::eval_insert_statement() {
    consume_token_of_type(TOKEN_INSERT);
    auto byte_code = eval_expression();
    consume_token_of_type(TOKEN_SEMI_COLON);

    return new InsertStatement(byte_code);
}

ReturnStatement* Parser::eval_return_statement() {
    consume_token_of_type(TOKEN_RETURN);
    return new ReturnStatement(eval_expression_statement()->expression);
}

BreakStatement* Parser::eval_break_statement() {
    // might just use an expression statement for this but for now it is a string lit
    consume_token_of_type(TOKEN_BREAK);
    auto identifier = *consume_token_of_type(TOKEN_STRING_LITERAL);
    consume_token_of_type(TOKEN_SEMI_COLON);
    return new BreakStatement(identifier);
}


ImportStatement* Parser::eval_import_statement() {
    consume_token_of_type(TOKEN_IMPORT);
    auto file = eval_expression_statement();

    return new ImportStatement(file->expression);
}

ExpressionStatement* Parser::eval_expression_statement() {
    auto expression = eval_expression();
    consume_token_of_type(TOKEN_SEMI_COLON);

    return new ExpressionStatement(expression);
}

AssigmentStatement* Parser::eval_assigment_statement() {
    auto identifier = consume_token_of_type(TOKEN_IDENTIFIER);
    consume_token_of_type(TOKEN_EQUAL);
    auto expression = eval_expression_statement();

    return new AssigmentStatement(*identifier, expression);
}

Expression* Parser::eval_expression() {
    return eval_term();
}

Expression* Parser::eval_term() {
    auto expr = eval_factor();

    while (match(TokenType::TOKEN_PLUS)) {
        Token* op = consume_token();
        auto right = eval_factor();
        expr = new BinaryExpression(expr, *op, right);
    }

    return expr;
}

Expression* Parser::eval_factor() {
    auto expr = eval_unary();

    while (match(TokenType::TOKEN_STAR)) {
        Token* op = consume_token();
        auto right = eval_unary();
        expr = new BinaryExpression(expr, *op, right);
    }

    return expr;
}

Expression* Parser::eval_unary() {  
    if (match(TOKEN_AT) || match(TOKEN_STAR)) {
        auto op = consume_token();
        auto expr = eval_unary();
        return new UnaryExpression(expr, *op);
    }

    return eval_call();
}

Expression* Parser::eval_call() {
    auto expr = eval_array();

    while (true) {
        if (match(TOKEN_PAREN_OPEN)) {
            consume_token_of_type(TOKEN_PAREN_OPEN);
            auto args = consume_arguments(TOKEN_PAREN_CLOSE);
            consume_token_of_type(TOKEN_PAREN_CLOSE);

            return new CallExpression(expr, args);
        }
        else if (match(TOKEN_DOT)) {
            consume_token();
            auto identifier = consume_token_of_type(TOKEN_IDENTIFIER);
            expr = new GetExpression(expr, *identifier);
        }
        else {
            break;
        }
    }

    return expr;
}

Expression* Parser::eval_array() {
    if (match(TOKEN_BRACKET_OPEN)) {
        consume_token_of_type(TOKEN_BRACKET_OPEN);
        auto args = consume_arguments(TOKEN_BRACKET_CLOSE);
        consume_token_of_type(TOKEN_BRACKET_CLOSE);

        return new ArrayExpression(args);
    }

    return eval_primary();
}

Expression* Parser::eval_primary() {
    auto type = peek()->type;

    if (type == TokenType::TOKEN_INT_LITERAL)
        return new IntLiteralExpression(*consume_token());
    else if (type == TokenType::TOKEN_STRING_LITERAL)
        return new StringLiteralExpression(*consume_token());
    else if (type == TokenType::TOKEN_IDENTIFIER)
        return new IdentifierExpression(*consume_token());
    else if (type == TokenType::TOKEN_NEW)
        return eval_new_expression();

    return new Expression(); // empty expression found -- like when a return has no expression
}

Expression* Parser::eval_new_expression() {
    consume_token();
    auto identifier = consume_token_of_type(TOKEN_IDENTIFIER);
    consume_token_of_type(TOKEN_BRACE_OPEN);
    auto expressions = consume_arguments(TOKEN_BRACE_CLOSE);
    consume_token_of_type(TOKEN_BRACE_CLOSE);
    return new NewExpression(*identifier, expressions);
}

TypeExpression* Parser::eval_type_expression() {
    switch (peek()->type)
    {
        case TOKEN_IDENTIFIER: return eval_identifier_type_expression(); break;
        case TOKEN_HAT: return eval_pointer_type_expression(); break;
        case TOKEN_BRACKET_OPEN: return eval_array_type_expression(); break;
        default:
            panic("Cannot parse token as the begining of a type expression");
            break;
    }
}

IdentifierTypeExpression* Parser::
eval_identifier_type_expression() {
    auto identifier = consume_token_of_type(TOKEN_IDENTIFIER);
    return new IdentifierTypeExpression(*identifier);
}

PointerTypeExpression* Parser::
eval_pointer_type_expression() {
    consume_token_of_type(TOKEN_HAT);
    auto pointer_of = eval_type_expression();
    return new PointerTypeExpression(pointer_of);
}

ArrayTypeExpression* Parser::
eval_array_type_expression() {
    consume_token_of_type(TOKEN_BRACKET_OPEN);
    auto array_of = eval_type_expression();
    consume_token_of_type(TOKEN_BRACKET_CLOSE);
    return new ArrayTypeExpression(array_of);
}

bool Parser::match(TokenType type) {
    if (tokens.size() > 0)
        return peek()->type == type;

    return false;
}

Token* Parser::peek(int offset) {
    return &tokens.at(current + offset);
}

Token* Parser::consume_token() {
    if (current >= tokens.size())
        panic("No more tokens to consume");

    return &tokens.at(current++);
}

Token* Parser::consume_token_of_type(TokenType type) {
    if (current >= tokens.size()) {
        std::ostringstream oss;
        oss << "Expected " << type << " but there are no more tokens to consume";
        panic(oss.str());
    }

    auto t_ptr = &tokens.at(current++);
    if (t_ptr->type != type) {
        unexpected_token(t_ptr, type);
    }

    return t_ptr;
}

std::vector<Expression*> Parser::consume_arguments(TokenType closer) {
    auto args = std::vector<Expression*>();
    bool is_first = true;
    if (!match(closer)) {
        do {
            if (!is_first) current++; // only iterate current by one when it is not the first time

            auto expr = eval_expression();
            args.push_back(expr);

            if (is_first) is_first = false;
        } while (match(TOKEN_COMMA));
    }

    return args;
}

std::vector<std::tuple<Token, TypeExpression*>> Parser::consume_comma_seperated_values() {
    auto args_types = std::vector<std::tuple<Token, TypeExpression*>>();
    bool is_first = true;
    if (!match(TOKEN_PAREN_CLOSE) && !match(TOKEN_BRACE_CLOSE)) {
        do {
            if (!is_first) current++; // only iterate current by one when it is not the first time

            auto arg = consume_token_of_type(TOKEN_IDENTIFIER);
            consume_token_of_type(TOKEN_COLON);
            auto type = eval_type_expression();

            args_types.emplace_back(*arg, type);

            if (is_first) is_first = false;
        } while (match(TOKEN_COMMA));
    }

    return args_types;
}

void unexpected_token(Token* got, TokenType expected) {
    std::ostringstream oss;
    oss << "Expected \'" << TokenTypeStrings[expected] << "\' got \'" << got->string << "\' at (" << got->line << ":" << got->character << ")";
    panic(oss.str());
}

void unexpected_eof() {
    panic("unexpexted end of file");
}