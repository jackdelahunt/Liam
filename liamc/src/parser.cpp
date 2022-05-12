#include "parser.h"
#include <tuple>
#include <utility>

File::File() {
    statements = std::vector<Statement*>();
}

Parser::Parser(std::string path, std::vector<Token>& tokens) {
    this->tokens = tokens;
    this->current = 0;
    this->root = File();
    this->path = path;
}

void Parser::parse() {
    while (current < tokens.size()) {
        auto [stmt, error] = eval_statement();
        if(error) {
            continue;
        }
        root.statements.push_back(stmt);
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

        auto lexer = Lexer(path);
        lexer.lex();

        auto other_parser = Parser(string_lit->token.string, lexer.tokens);
        other_parser.parse();

        for (auto stmt : root.statements) {
            other_parser.root.statements.push_back(stmt);
        }

        root = other_parser.root;
    }
}

std::tuple<Statement*, bool> Parser::
eval_statement() {
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
    case TOKEN_FOR:
        return eval_for_statement();
        break;
    case TOKEN_IDENTIFIER:
        // x := y; ..or.. x();
        if (peek(1)->type == TOKEN_EQUAL) {
            return eval_assigment_statement();
        }
    default:
        return eval_expression_statement();
        break;
    }
}

std::tuple<LetStatement*, bool> Parser::
eval_let_statement() {
    TRY_TOKEN(TOKEN_LET);
    NAMED_TOKEN(identifier, TOKEN_IDENTIFIER);

    // might be walrus or explicit type
    if(peek(0)->type == TOKEN_COLON) {
        TRY_TOKEN(TOKEN_COLON);
        TRY(TypeExpression*, type, eval_type_expression());
        TRY_TOKEN(TOKEN_EQUAL);
        TRY(ExpressionStatement*, expression, eval_expression_statement());
        return WIN(new LetStatement(*identifier, expression->expression, type));
    } else {
        TRY_TOKEN(TOKEN_WALRUS);
        TRY(ExpressionStatement*, expression, eval_expression_statement());
        return WIN(new LetStatement(*identifier, expression->expression, nullptr));
    }
}

std::tuple<ScopeStatement*, bool> Parser::
eval_scope_statement() {
    auto statements = std::vector<Statement*>();
    TRY_TOKEN(TOKEN_BRACE_OPEN);
    int closing_brace_index = find_balance_point(TOKEN_BRACE_OPEN, TOKEN_BRACE_CLOSE, current - 1);
    if (closing_brace_index == current + 1) { // if this scope is empty
        current++;
    }
    while (current < closing_brace_index) {
        TRY(Statement*, statement, eval_statement());
        statements.push_back(statement);
    }
    TRY_TOKEN(TOKEN_BRACE_CLOSE);

    return WIN(new ScopeStatement(statements));
}

std::tuple<FnStatement*, bool> Parser::
eval_fn_statement() {
    TRY_TOKEN(TOKEN_FN);
    NAMED_TOKEN(identifier, TOKEN_IDENTIFIER);
    TRY_TOKEN(TOKEN_PAREN_OPEN);
    auto [params, error] = consume_comma_seperated_values();
    if(error) {
        return {nullptr, true};
    }
    TRY_TOKEN(TOKEN_PAREN_CLOSE);
    TRY_TOKEN(TOKEN_COLON);

    TRY(TypeExpression*, type, eval_type_expression());
    TRY(ScopeStatement*, body, eval_scope_statement());

    return WIN(new FnStatement(*identifier, params, type, body));
}

std::tuple<LoopStatement*, bool> Parser::
eval_loop_statement() {
    TRY_TOKEN(TOKEN_LOOP);
    NAMED_TOKEN(identifier, TOKEN_STRING_LITERAL);
    TRY(ScopeStatement*, body, eval_scope_statement());
    return WIN(new LoopStatement(*identifier, body));
}

int Parser::
find_balance_point(TokenType push, TokenType pull, int from) {
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

std::tuple<StructStatement*, bool> Parser::
eval_struct_statement() {
    TRY_TOKEN(TOKEN_STRUCT);
    NAMED_TOKEN(identifier, TOKEN_IDENTIFIER);
    TRY_TOKEN(TOKEN_BRACE_OPEN);
    auto [member, error] = consume_comma_seperated_values();
    if(error) {
        return {nullptr, true};
    }
    TRY_TOKEN(TOKEN_BRACE_CLOSE);
    return WIN(new StructStatement(*identifier, member));
}

std::tuple<InsertStatement*, bool> Parser::
eval_insert_statement() {
    TRY_TOKEN(TOKEN_INSERT);
    TRY(Expression*, byte_code, eval_expression());
    TRY_TOKEN(TOKEN_SEMI_COLON);

    return WIN(new InsertStatement(byte_code));
}

std::tuple<ReturnStatement*, bool> Parser::
eval_return_statement() {
    TRY_TOKEN(TOKEN_RETURN);
    TRY(ExpressionStatement*, expression, eval_expression_statement());
    return WIN(new ReturnStatement(expression->expression));
}

std::tuple<BreakStatement*, bool> Parser::
eval_break_statement() {
    // might just use an expression statement for this but for now it is a string lit
    TRY_TOKEN(TOKEN_BREAK);
    NAMED_TOKEN(identifier, TOKEN_STRING_LITERAL);
    consume_token_of_type(TOKEN_SEMI_COLON);
    return WIN(new BreakStatement(*identifier));
}


std::tuple<ImportStatement*, bool> Parser::
eval_import_statement() {
    TRY_TOKEN(TOKEN_IMPORT);
    TRY(ExpressionStatement*, file, eval_expression_statement());

    return WIN(new ImportStatement(file->expression));
}

std::tuple<ForStatement*, bool> Parser::
eval_for_statement() {
    TRY_TOKEN(TOKEN_FOR);
    NAMED_TOKEN(value_token, TOKEN_IDENTIFIER);
    NAMED_TOKEN(index_token, TOKEN_IDENTIFIER);
    TRY_TOKEN(TOKEN_IN);
    TRY(Expression*, expression, eval_expression())
    TRY(ScopeStatement*, body, eval_scope_statement())

    return WIN(new ForStatement(expression, body, *value_token, *index_token));
}

std::tuple<ExpressionStatement*, bool> Parser::
eval_expression_statement() {
    TRY(Expression*, expression, eval_expression());
    TRY_TOKEN(TOKEN_SEMI_COLON)

    return WIN(new ExpressionStatement(expression));
}

std::tuple<AssigmentStatement*, bool> Parser::
eval_assigment_statement() {
    NAMED_TOKEN(identifier, TOKEN_IDENTIFIER);
    TRY_TOKEN(TOKEN_EQUAL);
    TRY(ExpressionStatement*, expression, eval_expression_statement());

    return WIN(new AssigmentStatement(*identifier, expression));
}

std::tuple<Expression*, bool> Parser::
eval_expression() {
    return eval_term();
}

std::tuple<Expression*, bool> Parser::
eval_term() {
    TRY(Expression*, expr, eval_factor());

    while (match(TokenType::TOKEN_PLUS)) {
        Token* op = consume_token();
        TRY(Expression*, right, eval_factor());
        expr = new BinaryExpression(expr, *op, right);
    }

    return WIN(expr);
}

std::tuple<Expression*, bool> Parser::
eval_factor() {
    TRY(Expression*, expr, eval_unary());

    while (match(TokenType::TOKEN_STAR)) {
        Token* op = consume_token();
        TRY(Expression*, right, eval_unary());
        expr = new BinaryExpression(expr, *op, right);
    }

    return {expr, false};
}

std::tuple<Expression*, bool> Parser::
eval_unary() {
    if (match(TOKEN_AT) || match(TOKEN_STAR)) {
        auto op = consume_token();
        TRY(Expression*, expr, eval_unary());
        return {new UnaryExpression(expr, *op), false};
    }

    return eval_postfix();
}

std::tuple<Expression*, bool> Parser::
eval_postfix() {
    TRY(Expression*, expr, eval_call());

    if (match(TOKEN_BRACKET_OPEN)) {
        TRY_TOKEN(TOKEN_BRACKET_OPEN);
        TRY(Expression*, expression, eval_expression());
        TRY_TOKEN(TOKEN_BRACKET_CLOSE);
        return {new ArraySubscriptExpression(expr, expression), false};
    }

    return {expr, false};
}

std::tuple<Expression*, bool> Parser::
eval_call() {
    auto [expr, error] = eval_array();
    if(error) {
        return {nullptr, true};
    }

    while (true) {
        if (match(TOKEN_PAREN_OPEN)) {
            TRY_TOKEN(TOKEN_PAREN_OPEN);
            auto [args, error] = consume_arguments(TOKEN_PAREN_CLOSE);
            if(error) {
                return {nullptr, true};
            }
            TRY_TOKEN(TOKEN_PAREN_CLOSE);

            return {new CallExpression(expr, args), false};
        }
        else if (match(TOKEN_DOT)) {
            consume_token();
            NAMED_TOKEN(identifier, TOKEN_IDENTIFIER);
            expr = new GetExpression(expr, *identifier);
        }
        else {
            break;
        }
    }

    return {expr, false};
}

std::tuple<Expression*, bool> Parser::
eval_array() {
    if (match(TOKEN_BRACKET_OPEN)) {
        TRY_TOKEN(TOKEN_BRACKET_OPEN);
        auto [args, error] = consume_arguments(TOKEN_BRACKET_CLOSE);
        if(error) {
            return {nullptr, true};
        }
        TRY_TOKEN(TOKEN_BRACKET_CLOSE);

        return {new ArrayExpression(args), false};
    }

    return eval_primary();
}

std::tuple<Expression*, bool> Parser::
eval_primary() {
    auto type = peek()->type;

    if (type == TokenType::TOKEN_INT_LITERAL)
        return {new IntLiteralExpression(*consume_token()), false};
    else if (type == TokenType::TOKEN_STRING_LITERAL)
        return {new StringLiteralExpression(*consume_token()), false};
    else if (type == TokenType::TOKEN_IDENTIFIER)
        return {new IdentifierExpression(*consume_token()), false};
    else if (type == TokenType::TOKEN_NEW)
        return eval_new_expression();

    return {new Expression(), false}; // empty expression found -- like when a return has no expression
}

std::tuple<Expression*, bool> Parser::
eval_new_expression() {
    consume_token();
    NAMED_TOKEN(identifier, TOKEN_IDENTIFIER);
    TRY_TOKEN(TOKEN_BRACE_OPEN);
    auto [expressions, error] = consume_arguments(TOKEN_BRACE_CLOSE);
    if(error) {
        return {nullptr, true};
    }
    TRY_TOKEN(TOKEN_BRACE_CLOSE);
    return WIN(new NewExpression(*identifier, expressions));
}

std::tuple<TypeExpression*, bool> Parser::eval_type_expression() {
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

std::tuple<IdentifierTypeExpression*, bool> Parser::
eval_identifier_type_expression() {
    NAMED_TOKEN(identifier, TOKEN_IDENTIFIER);
    return WIN(new IdentifierTypeExpression(*identifier));
}

std::tuple<PointerTypeExpression*, bool> Parser::
eval_pointer_type_expression() {
    TRY_TOKEN(TOKEN_HAT);
    TRY(TypeExpression*, pointer_of, eval_type_expression());
    return WIN(new PointerTypeExpression(pointer_of));
}

std::tuple<ArrayTypeExpression*, bool> Parser::
eval_array_type_expression() {
    TRY_TOKEN(TOKEN_BRACE_OPEN);
    TRY(TypeExpression*, array_of, eval_type_expression());
    TRY_TOKEN(TOKEN_BRACKET_CLOSE);
    return WIN(new ArrayTypeExpression(array_of));
}

void Parser::
report_error(int line, int character, std::string message) {
    ErrorReport report = {
            line,
            character,
            std::move(message)
    };

    errors.push_back(report);
}

bool Parser::
match(TokenType type) {
    if (tokens.size() > 0)
        return peek()->type == type;

    return false;
}

Token* Parser::
peek(int offset) {
    return &tokens.at(current + offset);
}

Token* Parser::
consume_token() {
    if (current >= tokens.size())
        panic("No more tokens to consume");

    return &tokens.at(current++);
}

std::tuple<Token*, bool> Parser::
consume_token_of_type(TokenType type) {
    if (current >= tokens.size()) {
        std::ostringstream oss;
        oss << "Expected " << type << " but there are no more tokens to consume";
        panic(oss.str());
    }

    auto t_ptr = &tokens.at(current++);
    if (t_ptr->type != type) {
        unexpected_token(t_ptr, type);
    }

    return {t_ptr, false};
}

void Parser::
unexpected_token(Token* got, TokenType expected) {
    // /home/jackdelahunt/Projects/Liam/liamc/src/parser.cpp:402:1
    report_error(got->line, got->character, path + ":"+ std::to_string(got->line) +
                                            ":" + std::to_string(got->character) + " :: Expected \'" +
                                            TokenTypeStrings[expected] +
                                            "\' got \'" + got->string +
                                            "\' at (" + std::to_string(got->line) +
                                            ":" + std::to_string(got->character) + ")"
                                            );
}

std::tuple<std::vector<Expression*>, bool> Parser::consume_arguments(TokenType closer) {
    auto args = std::vector<Expression*>();
    bool is_first = true;
    if (!match(closer)) {
        do {
            if (!is_first) current++; // only iterate current by one when it is not the first time

            auto [expr, error] = eval_expression();
            if(error) {
                return {{}, true};
            }

            args.push_back(expr);

            if (is_first) is_first = false;
        } while (match(TOKEN_COMMA));
    }

    return WIN(args);
}

std::tuple<CSV, bool> Parser::consume_comma_seperated_values() {
    auto args_types = std::vector<std::tuple<Token, TypeExpression*>>();
    bool is_first = true;
    if (!match(TOKEN_PAREN_CLOSE) && !match(TOKEN_BRACE_CLOSE)) {
        do {
            if (!is_first) current++; // only iterate current by one when it is not the first time

            auto [arg, identifier_error] = consume_token_of_type(TOKEN_IDENTIFIER);
            if(identifier_error) {
                return {{}, true};
            }

            auto [_, colon_error] = consume_token_of_type(TOKEN_COLON);
            if(colon_error) {
                return {{}, true};
            }

            auto [type, type_error] = eval_type_expression();
            if(type_error) {
                return {{}, true};
            }

            args_types.emplace_back(*arg, type);

            if (is_first) is_first = false;
        } while (match(TOKEN_COMMA));
    }

    return WIN(args_types);
}

void unexpected_eof() {
    panic("unexpexted end of file");
}