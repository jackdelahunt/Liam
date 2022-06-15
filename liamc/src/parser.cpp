#include "parser.h"
#include <tuple>
#include <utility>
#include "errors.h"
#include "liam.h"

File::
File(std::filesystem::path path) {
    statements = std::vector<Statement*>();
    imports = std::vector<std::string>();
    this->path = std::move(path);
}

Parser::
Parser(std::filesystem::path path, std::vector<Token>& tokens) {
    this->tokens = tokens;
    this->current = 0;
    this->path = std::move(path);
}

File Parser::
parse() {
    auto file = File(path);
    while (current < tokens.size()) {
        auto [stmt, error] = eval_statement();
        if(error) {
            continue;
        }

        if(stmt->statement_type == StatementType::STATEMENT_IMPORT) {
            auto import_stmt = dynamic_cast<ImportStatement*>(stmt);
            if(import_stmt->file->type != ExpressionType::EXPRESSION_STRING_LITERAL) {
                panic("Import requires string literal");
            }
            auto import_path = dynamic_cast<StringLiteralExpression*>(import_stmt->file);
            file.imports.emplace_back(absolute(std::filesystem::path(import_path->token.string)).string());
        } else {
            file.statements.push_back(stmt);
        }
    }

    return file;
}

std::tuple<Statement*, bool> Parser::
eval_statement() {
    switch (peek()->type)
    {
    case TOKEN_LET:
        return eval_let_statement();
        break;
    case TOKEN_OVERRIDE:
        return eval_override_statement();
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
    case TOKEN_IF:
        return eval_if_statement();
        break;
    default: 
        return eval_line_starting_expression();
        break;
    }
}

std::tuple<LetStatement*, bool> Parser::
eval_let_statement() {
    TRY_TOKEN(TOKEN_LET);
    NAMED_TOKEN(identifier, TOKEN_IDENTIFIER);

    // might be walrus or explicit return_type
    if(peek(0)->type == TOKEN_COLON) {
        TRY_TOKEN(TOKEN_COLON);
        TRY(TypeExpression*, type, eval_type_expression());
        TRY_TOKEN(TOKEN_ASSIGN);
        TRY(ExpressionStatement*, expression, eval_expression_statement());
        return WIN(new LetStatement(*identifier, expression->expression, type));
    } else {
        TRY_TOKEN(TOKEN_WALRUS);
        TRY(ExpressionStatement*, expression, eval_expression_statement());

        return WIN(new LetStatement(*identifier, expression->expression, nullptr));
    }
}

std::tuple<OverrideStatement*, bool> Parser::
eval_override_statement() {
    TRY_TOKEN(TOKEN_OVERRIDE);
    NAMED_TOKEN(identifier, TOKEN_IDENTIFIER);

    TRY_TOKEN(TOKEN_COLON);
    TRY(TypeExpression*, type, eval_type_expression());
    TRY_TOKEN(TOKEN_ASSIGN);
    TRY(ExpressionStatement*, expression, eval_expression_statement());
    return WIN(new OverrideStatement(*identifier, expression->expression, type));
}

std::tuple<ScopeStatement*, bool> Parser::
eval_scope_statement() {
    auto statements = std::vector<Statement*>();
    NAMED_TOKEN(open_brace, TOKEN_BRACE_OPEN);
    s32 closing_brace_index = find_balance_point(TOKEN_BRACE_OPEN, TOKEN_BRACE_CLOSE, current - 1);
    if (closing_brace_index == current + 1) { // if this scope is empty
        current++;
    } else if(closing_brace_index < 0) {
        FAIL(path.string(), open_brace->line, open_brace->character, "No closing brace for scope found");
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

    auto generics = std::vector<Token>();
    if(peek()->type == TokenType::TOKEN_LESS) {
        TRY_TOKEN(TOKEN_LESS);
        auto [types, error] = consume_comma_seperated_token_arguments(TOKEN_GREATER);
        if(error) {
            return {nullptr, true};
        }
        TRY_TOKEN(TOKEN_GREATER);

        generics = types;
    }

    TRY_TOKEN(TOKEN_PAREN_OPEN);
    auto [params, error] = consume_comma_seperated_params();
    if(error) {
        return {nullptr, true};
    }
    TRY_TOKEN(TOKEN_PAREN_CLOSE);
    TRY_TOKEN(TOKEN_COLON);

    TRY(TypeExpression*, type, eval_type_expression());
    TRY(ScopeStatement*, body, eval_scope_statement());

    return WIN(new FnStatement(*identifier, generics, params, type, body));
}

std::tuple<LoopStatement*, bool> Parser::
eval_loop_statement() {
    TRY_TOKEN(TOKEN_LOOP);
    NAMED_TOKEN(identifier, TOKEN_STRING_LITERAL);
    TRY(ScopeStatement*, body, eval_scope_statement());
    return WIN(new LoopStatement(*identifier, body));
}

s32 Parser::
find_balance_point(TokenType push, TokenType pull, s32 from) {
    s32 current_index = from;
    s32 balance = 0;

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
    auto [member, error] = consume_comma_seperated_params();
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
    TRY(LetStatement*, let_statement, eval_let_statement())
    TRY(Expression*, condition, eval_expression())
    TRY_TOKEN(TOKEN_SEMI_COLON);
    TRY(Expression*, update, eval_expression())
    TRY_TOKEN(TOKEN_SEMI_COLON);
    TRY(ScopeStatement*, body, eval_scope_statement())

    return WIN(new ForStatement(let_statement, condition, update, body));
}

std::tuple<IfStatement*, bool> Parser::
eval_if_statement() {
    TRY_TOKEN(TOKEN_IF);
    TRY(Expression*, expression, eval_expression())
    TRY(ScopeStatement*, body, eval_scope_statement())
    return WIN(new IfStatement(expression, body));
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
    TRY_TOKEN(TOKEN_ASSIGN);
    TRY(ExpressionStatement*, expression, eval_expression_statement());

    return WIN(new AssigmentStatement(*identifier, expression));
}

std::tuple<Statement*, bool> Parser::eval_line_starting_expression() {
    auto expr = eval_expression();
    if (peek()->type == TOKEN_ASSIGN) {
        TRY_TOKEN(TOKEN_ASSIGN);
        TRY(ExpressionStatement*, expression, eval_expression_statement());

        return WIN(new AssigmentStatement(*identifier, expression));
    } else {
        return eval_expression_statement();
    }
        break;
    }
}

std::tuple<Expression*, bool> Parser::
eval_expression() {
    return eval_or();
}

std::tuple<Expression*, bool> Parser::
eval_or() {
    TRY(Expression*, expr, eval_and());

    while (match(TokenType::TOKEN_OR)) {
        Token* op = consume_token();
        TRY(Expression*, right, eval_expression());
        expr = new BinaryExpression(expr, *op, right);
    }

    return WIN(expr);
}

std::tuple<Expression*, bool> Parser::
eval_and() {
    TRY(Expression*, expr, eval_comparison());

    while (match(TokenType::TOKEN_AND)) {
        Token* op = consume_token();
        TRY(Expression*, right, eval_expression());
        expr = new BinaryExpression(expr, *op, right);
    }

    return WIN(expr);
}

std::tuple<Expression*, bool> Parser::
eval_comparison() {
    TRY(Expression*, expr, eval_term());

    while (match(TokenType::TOKEN_NOT_EQUAL) || match(TokenType::TOKEN_EQUAL)) {
        Token* op = consume_token();
        TRY(Expression*, right, eval_expression());
        expr = new BinaryExpression(expr, *op, right);
    }

    return WIN(expr);
}

std::tuple<Expression*, bool> Parser::
eval_term() {
    TRY(Expression*, expr, eval_factor());

    while (match(TokenType::TOKEN_PLUS)) {
        Token* op = consume_token();
        TRY(Expression*, right, eval_expression());
        expr = new BinaryExpression(expr, *op, right);
    }

    return WIN(expr);
}

std::tuple<Expression*, bool> Parser::
eval_factor() {
    TRY(Expression*, expr, eval_unary());

    while (match(TokenType::TOKEN_STAR)) {
        Token* op = consume_token();
        TRY(Expression*, right, eval_expression());
        expr = new BinaryExpression(expr, *op, right);
    }

    return {expr, false};
}

std::tuple<Expression*, bool> Parser::
eval_unary() {
    if (match(TOKEN_AT) || match(TOKEN_STAR)) {
        auto op = consume_token();
        TRY(Expression*, expr, eval_expression());
        return {new UnaryExpression(expr, *op), false};
    }

    return eval_call();
}


std::tuple<Expression*, bool> Parser::
eval_call() {
    TRY(Expression*, expr, eval_primary());

    while (true) {
        if (match(TOKEN_PAREN_OPEN) || match(TOKEN_LESS)) {

            auto generics = std::vector<TypeExpression*>();
            if(peek()->type == TOKEN_LESS) {
                TRY_TOKEN(TOKEN_LESS);
                auto [types, error] = consume_comma_seperated_types(TOKEN_GREATER);
                if(error) {
                    return {nullptr, true};
                }
                TRY_TOKEN(TOKEN_GREATER);

                generics = types;
            }

            TRY_TOKEN(TOKEN_PAREN_OPEN);
            auto [args, error] = consume_comma_seperated_arguments(TOKEN_PAREN_CLOSE);
            if(error) {
                return {nullptr, true};
            }
            TRY_TOKEN(TOKEN_PAREN_CLOSE);

            return {new CallExpression(expr, args, generics), false};
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
eval_primary() {
    auto type = peek()->type;

    if (type == TokenType::TOKEN_INT_LITERAL)
        return WIN(new IntLiteralExpression(*consume_token()));
    else if (type == TokenType::TOKEN_STRING_LITERAL)
        return WIN(new StringLiteralExpression(*consume_token()));
    else if (type == TokenType::TOKEN_TRUE || type == TokenType::TOKEN_FALSE)
        return WIN(new BoolLiteralExpression(*consume_token()));
    else if (type == TokenType::TOKEN_IDENTIFIER)
        return WIN(new IdentifierExpression(*consume_token()));
    else if (type == TokenType::TOKEN_NEW)
        return eval_new_expression();
    else if (type == TokenType::TOKEN_PAREN_OPEN) {
        return eval_group_expression();
    }

    return WIN(new Expression()); // empty expression found -- like when a return has no expression
}

std::tuple<Expression*, bool> Parser::
eval_new_expression() {
    consume_token();
    NAMED_TOKEN(identifier, TOKEN_IDENTIFIER);
    TRY_TOKEN(TOKEN_BRACE_OPEN);
    auto [expressions, error] = consume_comma_seperated_arguments(TOKEN_BRACE_CLOSE);
    if(error) {
        return {nullptr, true};
    }
    TRY_TOKEN(TOKEN_BRACE_CLOSE);
    return WIN(new NewExpression(*identifier, expressions));
}

std::tuple<Expression*, bool> Parser::
eval_group_expression() {
    TRY_TOKEN(TOKEN_PAREN_OPEN);
    TRY(Expression*, expr, eval_expression());
    TRY_TOKEN(TOKEN_PAREN_CLOSE);
    return WIN(new GroupExpression(expr));
}

std::tuple<TypeExpression*, bool> Parser::
eval_type_expression() {
    switch (peek()->type)
    {
        case TOKEN_IDENTIFIER: return eval_identifier_type_expression(); break;
        case TOKEN_HAT: return eval_pointer_type_expression(); break;
        default:
            FAIL(path.string(), peek()->line, peek()->character, "Cannot parse token as the beginning of a return_type expression \'" +
                    peek()->string + "\'");
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

bool Parser::
match(TokenType type) {
    if (tokens.size() > 0)
        return peek()->type == type;

    return false;
}

Token* Parser::
peek(s32 offset) {
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
        auto last_token = tokens.at(tokens.size() - 1);
        FAIL(path.string(), last_token.line, last_token.character,
             std::string("Expected \'") + TokenTypeStrings[type] + std::string("\' but got unexpected end of file"));
    }

    auto t_ptr = &tokens.at(current++);
    if (t_ptr->type != type) {
        FAIL(path.string(), t_ptr->line, t_ptr->character,
             std::string("Expected \'") +
             TokenTypeStrings[type] +
             "\' got \'" + t_ptr->string +
             "\' at (" + std::to_string(t_ptr->line) +
             ":" + std::to_string(t_ptr->character) + ")");
    }

    return {t_ptr, false};
}

// e.g. (0, "hello sailor", ...)
std::tuple<std::vector<Expression*>, bool> Parser::
consume_comma_seperated_arguments(TokenType closer) {
    auto args = std::vector<Expression*>();
    bool is_first = true;
    if (!match(closer)) {
        do {
            if (!is_first) current++; // only iterate current by one when it is not the first time

            auto [expr, error] = eval_expression();
            if(error) {
                return {std::vector<Expression*>(), true};
            }

            args.push_back(expr);

            if (is_first) is_first = false;
        } while (match(TOKEN_COMMA));
    }

    return WIN(args);
}

// e.g. (X, Y, Z, ...)
std::tuple<std::vector<Token>, bool> Parser::
consume_comma_seperated_token_arguments(TokenType closer) {
    auto args = std::vector<Token>();
    bool is_first = true;
    if (!match(closer)) {
        do {
            if (!is_first) current++; // only iterate current by one when it is not the first time

            auto token = consume_token();
            args.push_back(*token);

            if (is_first) is_first = false;
        } while (match(TOKEN_COMMA));
    }

    return WIN(args);
}

// e.g. (int32, ^char, ...)
std::tuple<std::vector<TypeExpression*>, bool> Parser::
consume_comma_seperated_types(TokenType closer) {
    auto types = std::vector<TypeExpression*>();
    bool is_first = true;
    if (!match(closer)) {
        do {
            if (!is_first) current++; // only iterate current by one when it is not the first time

            auto [type, error] = eval_type_expression();
            if(error) {
                return {std::vector<TypeExpression*>(), true};
            }

            types.push_back(type);

            if (is_first) is_first = false;
        } while (match(TOKEN_COMMA));
    }

    return WIN(types);
}

// e.g. (int x, int y, ...)
std::tuple<CSV, bool> Parser::
consume_comma_seperated_params() {
    auto args_types = std::vector<std::tuple<Token, TypeExpression*>>();
    bool is_first = true;
    if (!match(TOKEN_PAREN_CLOSE) && !match(TOKEN_BRACE_CLOSE)) {
        do {
            if (!is_first) current++; // only iterate current by one when it is not the first time

            auto [arg, identifier_error] = consume_token_of_type(TOKEN_IDENTIFIER);
            if(identifier_error) {
                return {CSV(), true};
            }

            auto [_, colon_error] = consume_token_of_type(TOKEN_COLON);
            if(colon_error) {
                return {CSV(), true};
            }

            auto [type, type_error] = eval_type_expression();
            if(type_error) {
                return {CSV(), true};
            }

            args_types.emplace_back(*arg, type);

            if (is_first) is_first = false;
        } while (match(TOKEN_COMMA));
    }

    return WIN(args_types);
}