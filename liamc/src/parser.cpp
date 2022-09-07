#include "parser.h"

#include <tuple>
#include <utility>

#include "args.h"
#include "errors.h"
#include "fmt/core.h"
#include "liam.h"
#include "utils.h"

File::File(std::filesystem::path path) {
    statements = std::vector<Statement *>();
    imports    = std::vector<std::string>();
    this->path = std::move(path);
}

Parser::Parser(std::filesystem::path path, std::vector<Token> &tokens) {
    this->tokens  = tokens;
    this->current = 0;
    this->path    = absolute(std::filesystem::path(path));
    this->file    = NULL;
}

void Parser::parse() {
    this->file = new File(path);
    while (current < tokens.size())
    {

        auto errors_before = ErrorReporter::error_count();
        auto stmt          = eval_statement();
        if (ErrorReporter::error_count() > errors_before)
            continue;

        if (stmt->statement_type == StatementType::STATEMENT_IMPORT)
        {
            auto import_stmt = dynamic_cast<ImportStatement *>(stmt);

            if (import_stmt->path->type != ExpressionType::EXPRESSION_STRING_LITERAL)
            { panic("Import requires string literal"); }

            auto import_path_expr = dynamic_cast<StringLiteralExpression *>(import_stmt->path);

            std::string final_path;
            auto import_path = std::filesystem::path(import_path_expr->token.string);

            if (import_path.is_absolute())
            { final_path = import_path_expr->token.string; }
            else if (import_path.string().rfind("stdlib/", 0) == 0)
            {
                final_path =
                    absolute(std::filesystem::path(args->stdlib)).string() + "/" + import_path.filename().string();
            }
            else
            { final_path = this->path.parent_path().string() + "/" + import_path.string(); }

            file->imports.emplace_back(final_path);
        }
        else
        { file->statements.push_back(stmt); }
    }
}

Statement *Parser::eval_statement() {
    switch (peek()->type)
    {
    case TokenType::TOKEN_LET:
        return eval_let_statement();
        break;
    case TokenType::TOKEN_FN:
        return eval_fn_statement();
        break;
    case TokenType::TOKEN_STRUCT:
        return eval_struct_statement();
        break;
    case TokenType::TOKEN_INSERT:
        return eval_insert_statement();
        break;
    case TokenType::TOKEN_RETURN:
        return eval_return_statement();
        break;
    case TokenType::TOKEN_BREAK:
        return eval_break_statement();
        break;
    case TokenType::TOKEN_IMPORT:
        return eval_import_statement();
        break;
    case TokenType::TOKEN_FOR:
        return eval_for_statement();
        break;
    case TokenType::TOKEN_IF:
        return eval_if_statement();
        break;
    case TokenType::TOKEN_EXTERN:
        return eval_extern_statement();
        break;
    case TokenType::TOKEN_ENUM:
        return eval_enum_statement();
        break;
    case TokenType::TOKEN_CONTINUE:
        return eval_continue_statement();
        break;
    case TokenType::TOKEN_ALIAS:
        return eval_alias_statement();
        break;
    case TokenType::TOKEN_TEST:
        return eval_test_statement();
        break;
    default:
        return eval_line_starting_expression();
        break;
    }
}

LetStatement *Parser::eval_let_statement() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_LET));
    auto identifier = TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_IDENTIFIER));

    TypeExpression *type = NULL;

    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_COLON));
    if (peek()->type != TokenType::TOKEN_ASSIGN)
    { type = TRY_CALL_RET(eval_type_expression()); }
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_ASSIGN));
    auto expression = TRY_CALL_RET(eval_expression_statement());
    return new LetStatement(file, *identifier, expression->expression, type);
}

ScopeStatement *Parser::eval_scope_statement() {
    auto statements = std::vector<Statement *>();
    auto open_brace = TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACE_OPEN));
    s32 closing_brace_index =
        find_balance_point(TokenType::TOKEN_BRACE_OPEN, TokenType::TOKEN_BRACE_CLOSE, current - 1);
    if (closing_brace_index == current + 1)
    { // if this scope is empty
        current++;
    }
    else if (closing_brace_index < 0)
    {
        ErrorReporter::report_parser_error(
            path.string(), open_brace->span.start, open_brace->span.start, "No closing brace for scope found"
        );
        return NULL;
    }

    while (current < closing_brace_index)
    {
        auto statement = TRY_CALL_RET(eval_statement());
        statements.push_back(statement);
    }
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACE_CLOSE));

    return new ScopeStatement(file, statements);
}

FnStatement *Parser::eval_fn_statement(bool is_extern) {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_FN));
    auto identifier = TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_IDENTIFIER));

    auto generics = std::vector<Token>();
    if (peek()->type == TokenType::TOKEN_BRACKET_OPEN)
    {
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACKET_OPEN));
        auto types = TRY_CALL_RET(consume_comma_seperated_token_arguments(TokenType::TOKEN_BRACKET_CLOSE));
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACKET_CLOSE));

        generics = types;
    }

    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_PAREN_OPEN));

    auto params = TRY_CALL_RET(consume_comma_seperated_params());
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_PAREN_CLOSE));
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_COLON));

    auto type = TRY_CALL_RET(eval_type_expression());

    if (is_extern)
    {
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_SEMI_COLON));
        return new FnStatement(file, *identifier, generics, params, type, NULL, true);
    }
    else
    {
        auto body = TRY_CALL_RET(eval_scope_statement());
        return new FnStatement(file, *identifier, generics, params, type, body, false);
    }
}

StructStatement *Parser::eval_struct_statement(bool is_extern) {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_STRUCT));

    auto identifier = TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_IDENTIFIER));

    auto generics = std::vector<Token>();
    if (peek()->type == TokenType::TOKEN_BRACKET_OPEN)
    {
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACKET_OPEN));

        auto types = TRY_CALL_RET(consume_comma_seperated_token_arguments(TokenType::TOKEN_BRACKET_CLOSE));
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACKET_CLOSE));
        generics = types;
    }

    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACE_OPEN));

    auto member = TRY_CALL_RET(consume_comma_seperated_params());
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACE_CLOSE));
    return new StructStatement(file, *identifier, generics, member, is_extern);
}

InsertStatement *Parser::eval_insert_statement() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_INSERT));
    auto byte_code = TRY_CALL_RET(eval_expression());

    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_SEMI_COLON));

    return new InsertStatement(file, byte_code);
}

ReturnStatement *Parser::eval_return_statement() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_RETURN));

    Expression *expression = NULL;

    if (peek()->type != TokenType::TOKEN_SEMI_COLON)
    {
        auto expression_statement = TRY_CALL_RET(eval_expression_statement());

        expression = expression_statement->expression;
    }
    else
    { TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_SEMI_COLON)); }

    return new ReturnStatement(file, expression);
}

BreakStatement *Parser::eval_break_statement() {
    // might just use an expression statement for this but for now it is a
    // string lit
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BREAK));
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_SEMI_COLON));
    return new BreakStatement(file);
}

ImportStatement *Parser::eval_import_statement() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_IMPORT));
    auto import_path = TRY_CALL_RET(eval_expression_statement());

    return new ImportStatement(file, import_path->expression);
}

ForStatement *Parser::eval_for_statement() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_FOR));

    auto assign = TRY_CALL_RET(eval_statement());

    auto condition = TRY_CALL_RET(eval_expression());

    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_SEMI_COLON));
    auto update = TRY_CALL_RET(eval_statement());

    auto body = TRY_CALL_RET(eval_scope_statement());

    return new ForStatement(file, assign, condition, update, body);
}

IfStatement *Parser::eval_if_statement() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_IF));
    auto expression = TRY_CALL_RET(eval_expression());
    auto body       = TRY_CALL_RET(eval_scope_statement());

    // next statement might be else so check if the next token is an 'else'
    // if so capture it and own it else just leave the else statement as NULL
    ElseStatement *else_statement = NULL;
    if (peek()->type == TokenType::TOKEN_ELSE)
    { else_statement = TRY_CALL_RET(eval_else_statement()); }

    return new IfStatement(file, expression, body, else_statement);
}

ElseStatement *Parser::eval_else_statement() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_ELSE));

    // check if it is an else if
    if (peek()->type == TokenType::TOKEN_IF)
    {
        auto if_statement = TRY_CALL_RET(eval_if_statement());
        return new ElseStatement(if_statement, NULL);
    }

    auto body = TRY_CALL_RET(eval_scope_statement());
    return new ElseStatement(NULL, body);
}

ExpressionStatement *Parser::eval_expression_statement() {
    auto expression = TRY_CALL_RET(eval_expression());
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_SEMI_COLON));

    return new ExpressionStatement(file, expression);
}

Statement *Parser::eval_extern_statement() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_EXTERN));

    if (peek()->type == TokenType::TOKEN_FN)
    { return eval_fn_statement(true); }

    if (peek()->type == TokenType::TOKEN_STRUCT)
    { return eval_struct_statement(true); }

    panic("Cannot extern this statement");
    return NULL;
}

EnumStatement *Parser::eval_enum_statement() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_ENUM));

    auto identifier = TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_IDENTIFIER));
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACE_OPEN));
    auto instances = TRY_CALL_RET(consume_comma_seperated_token_arguments(TokenType::TOKEN_BRACE_CLOSE));
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACE_CLOSE));

    return new EnumStatement(file, *identifier, instances);
}

ContinueStatement *Parser::eval_continue_statement() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_CONTINUE));
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_SEMI_COLON));
    return new ContinueStatement(file);
}

AliasStatement *Parser::eval_alias_statement() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_ALIAS));
    auto identifier = TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_IDENTIFIER));
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_AS));
    auto type_expression = TRY_CALL_RET(eval_type_expression());
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_SEMI_COLON));

    return new AliasStatement(file, *identifier, type_expression);
}

TestStatement *Parser::eval_test_statement() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_TEST));

    auto tests = TRY_CALL_RET(eval_scope_statement());
    return new TestStatement(file, tests);
}

Statement *Parser::eval_line_starting_expression() {
    auto lhs = TRY_CALL_RET(eval_expression());

    if (peek()->type == TokenType::TOKEN_ASSIGN)
    {
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_ASSIGN));
        auto rhs = TRY_CALL_RET(eval_expression_statement());

        return new AssigmentStatement(file, lhs, rhs);
    }

    // not assign, after eval expresion only semi colon is left
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_SEMI_COLON));
    return new ExpressionStatement(file, lhs);
}

/*
 *  === expression precedence === (lower is more precedence)
 *  is
 *  ?
 *  or
 *  and
 *  == !=
 *  < > >= <=
 *  + -
 *  * / %
 *  @ * !
 *  call()
 *  literal () new "" null true false
 */

Expression *Parser::eval_expression() {
    return eval_is();
}

Expression *Parser::eval_is() {
    auto expr = TRY_CALL_RET(eval_propagation());

    if (match(TokenType::TOKEN_IS))
    {
        consume_token();
        auto type = TRY_CALL_RET(eval_type_expression());
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_AS));
        auto identifier = TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_IDENTIFIER));

        expr = new IsExpression(expr, type, *identifier);
    }

    return expr;
}

Expression *Parser::eval_propagation() {
    auto expr = TRY_CALL_RET(eval_or());

    if (match(TokenType::TOKEN_RETURN))
    {
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_RETURN));
        auto type = TRY_CALL_RET(eval_type_expression());
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_ELSE));
        auto otherwise = TRY_CALL_RET(eval_type_expression());
        expr           = new PropagateExpression(expr, type, otherwise);
    }

    return expr;
}

Expression *Parser::eval_or() {
    auto expr = TRY_CALL_RET(eval_and());

    while (match(TokenType::TOKEN_OR))
    {
        Token *op  = consume_token();
        auto right = TRY_CALL_RET(eval_and());
        expr       = new BinaryExpression(expr, *op, right);
    }

    return expr;
}

Expression *Parser::eval_and() {
    auto expr = TRY_CALL_RET(eval_equality());

    while (match(TokenType::TOKEN_AND))
    {
        Token *op  = consume_token();
        auto right = TRY_CALL_RET(eval_equality());
        expr       = new BinaryExpression(expr, *op, right);
    }

    return expr;
}

Expression *Parser::eval_equality() {
    auto expr = TRY_CALL_RET(eval_relational());

    while (match(TokenType::TOKEN_NOT_EQUAL) || match(TokenType::TOKEN_EQUAL))
    {
        Token *op  = consume_token();
        auto right = TRY_CALL_RET(eval_relational());
        expr       = new BinaryExpression(expr, *op, right);
    }

    return expr;
}

Expression *Parser::eval_relational() {
    auto expr = TRY_CALL_RET(eval_term());

    while (match(TokenType::TOKEN_LESS) || match(TokenType::TOKEN_GREATER) || match(TokenType::TOKEN_GREATER_EQUAL) ||
           match(TokenType::TOKEN_LESS_EQUAL))
    {
        Token *op  = consume_token();
        auto right = TRY_CALL_RET(eval_term());
        expr       = new BinaryExpression(expr, *op, right);
    }

    return expr;
}

Expression *Parser::eval_term() {
    auto expr = TRY_CALL_RET(eval_factor());

    while (match(TokenType::TOKEN_PLUS) || match(TokenType::TOKEN_MINUS))
    {
        Token *op  = consume_token();
        auto right = TRY_CALL_RET(eval_factor());
        expr       = new BinaryExpression(expr, *op, right);
    }

    return expr;
}

Expression *Parser::eval_factor() {
    auto expr = TRY_CALL_RET(eval_unary());

    while (match(TokenType::TOKEN_STAR) || match(TokenType::TOKEN_SLASH) || match(TokenType::TOKEN_MOD))
    {
        Token *op  = consume_token();
        auto right = TRY_CALL_RET(eval_unary());
        expr       = new BinaryExpression(expr, *op, right);
    }

    return expr;
}

Expression *Parser::eval_unary() {
    if (match(TokenType::TOKEN_AT) || match(TokenType::TOKEN_STAR) || match(TokenType::TOKEN_NOT))
    {
        auto op   = consume_token();
        auto expr = TRY_CALL_RET(eval_unary());

        return new UnaryExpression(expr, *op);
    }

    return eval_call();
}

Expression *Parser::eval_call() {
    auto expr = TRY_CALL_RET(eval_primary());

    while (true)
    {
        if (match(TokenType::TOKEN_PAREN_OPEN) || match(TokenType::TOKEN_BRACKET_OPEN))
        {
            auto generics = std::vector<TypeExpression *>();
            if (peek()->type == TokenType::TOKEN_BRACKET_OPEN)
            {
                TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACKET_OPEN));
                auto types = TRY_CALL_RET(consume_comma_seperated_types(TokenType::TOKEN_BRACKET_CLOSE));
                TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACKET_CLOSE));

                generics = types;
            }

            TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_PAREN_OPEN));
            auto call_args = TRY_CALL_RET(consume_comma_seperated_arguments(TokenType::TOKEN_PAREN_CLOSE));
            TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_PAREN_CLOSE));

            expr = new CallExpression(expr, call_args, generics);
        }
        else if (match(TokenType::TOKEN_DOT))
        {
            consume_token();
            auto identifier = TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_IDENTIFIER));
            expr            = new GetExpression(expr, *identifier);
        }
        else
        { break; }
    }

    return expr;
}

Expression *Parser::eval_primary() {
    auto type = peek()->type;

    if (type == TokenType::TOKEN_NUMBER_LITERAL)
        return new NumberLiteralExpression(*consume_token());
    else if (type == TokenType::TOKEN_STRING_LITERAL)
        return new StringLiteralExpression(*consume_token());
    else if (type == TokenType::TOKEN_TRUE || type == TokenType::TOKEN_FALSE)
        return new BoolLiteralExpression(*consume_token());
    else if (type == TokenType::TOKEN_IDENTIFIER)
        return new IdentifierExpression(*consume_token());
    else if (type == TokenType::TOKEN_NEW)
        return eval_new_expression();
    else if (type == TokenType::TOKEN_PAREN_OPEN)
        return eval_group_expression();
    else if (type == TokenType::TOKEN_NULL)
        return new NullLiteralExpression(*consume_token());
    else if (type == TokenType::TOKEN_ZERO)
        return new ZeroLiteralExpression(*consume_token());

    return new Expression(); // empty expression found -- like when a
                             // return has no expression
}

Expression *Parser::eval_new_expression() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_NEW));
    auto identifier = TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_IDENTIFIER));

    auto generics = std::vector<TypeExpression *>();
    if (peek()->type == TokenType::TOKEN_BRACKET_OPEN)
    {
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACKET_OPEN));
        auto types = TRY_CALL_RET(consume_comma_seperated_types(TokenType::TOKEN_BRACKET_CLOSE));
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACKET_CLOSE));

        generics = types;
    }

    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACE_OPEN));
    auto named_expressions = TRY_CALL_RET(consume_comma_seperated_named_arguments(TokenType::TOKEN_BRACE_CLOSE));
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACE_CLOSE));

    return new NewExpression(*identifier, generics, named_expressions);
}

Expression *Parser::eval_group_expression() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_PAREN_OPEN));
    auto expr = TRY_CALL_RET(eval_expression());
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_PAREN_CLOSE));
    return new GroupExpression(expr);
}

/*
 *  === type expression precedence === (lower is more precedence)
 * |
 * ^
 * []
 * identifier
 */
TypeExpression *Parser::eval_type_expression() {
    return eval_type_union();
}

TypeExpression *Parser::eval_type_union() {
    auto type_expression = TRY_CALL_RET(eval_type_unary());

    // there is a bar then this is a union type else return normal type
    if (match(TokenType::TOKEN_BAR))
    {
        auto expressions = std::vector<TypeExpression *>();
        expressions.push_back(type_expression);
        while (match(TokenType::TOKEN_BAR))
        {
            TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BAR));
            auto next_expression = TRY_CALL_RET(eval_type_unary());
            expressions.push_back(next_expression);
        }

        return new UnionTypeExpression(expressions);
    }

    return type_expression;
}

TypeExpression *Parser::eval_type_unary() {
    if (match(TokenType::TOKEN_HAT))
    {
        Token *op            = consume_token();
        auto type_expression = TRY_CALL_RET(eval_type_unary());
        return new UnaryTypeExpression(*op, type_expression);
    }

    return eval_type_specified_generics();
}

TypeExpression *Parser::eval_type_specified_generics() {
    auto struct_type = TRY_CALL_RET(eval_type_identifier());

    if (match(TokenType::TOKEN_BRACKET_OPEN))
    {
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACKET_OPEN));
        auto generics = TRY_CALL_RET(consume_comma_seperated_types(TokenType::TOKEN_BRACKET_CLOSE));
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACKET_CLOSE));

        return new SpecifiedGenericsTypeExpression(struct_type, generics);
    }

    return struct_type;
}

IdentifierTypeExpression *Parser::eval_type_identifier() {
    auto identifier = TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_IDENTIFIER));
    return new IdentifierTypeExpression(*identifier);
}

s32 Parser::find_balance_point(TokenType push, TokenType pull, s32 from) {
    s32 current_index = from;
    s32 balance       = 0;

    while (current_index < tokens.size())
    {
        if (tokens.at(current_index).type == push)
        {
            balance++;
            if (balance == 0)
                return current_index;
        }
        if (tokens.at(current_index).type == pull)
        {
            balance--;
            if (balance == 0)
                return current_index;
        }

        current_index++;
    }

    return -1;
}

bool Parser::match(TokenType type) {
    if (tokens.size() > 0)
        return peek()->type == type;

    return false;
}

Token *Parser::peek(s32 offset) {
    return &tokens.at(current + offset);
}

Token *Parser::consume_token() {
    if (current >= tokens.size())
        panic("No more tokens to consume");

    return &tokens.at(current++);
}

Token *Parser::consume_token_of_type(TokenType type) {
    if (current >= tokens.size())
    {
        auto last_token = tokens.at(tokens.size() - 1);
        ErrorReporter::report_parser_error(
            path.string(), last_token.span.line, last_token.span.start,
            fmt::format("Expected '{}' but got unexpected end of file", TokenTypeStrings[(int)type])
        );
        return NULL;
    }

    auto t_ptr = &tokens.at(current++);
    if (t_ptr->type != type)
    {
        ErrorReporter::report_parser_error(
            path.string(), t_ptr->span.line, t_ptr->span.start,
            fmt::format(
                "Expected '{}' got '{}' at({}:{})", TokenTypeStrings[(int)type], t_ptr->string, t_ptr->span.line,
                t_ptr->span.start
            )
        );
        return NULL;
    }

    return t_ptr;
}

// e.g. (0, "hello sailor", ...)
std::vector<Expression *> Parser::consume_comma_seperated_arguments(TokenType closer) {
    auto args     = std::vector<Expression *>();
    bool is_first = true;
    if (!match(closer))
    {
        do
        {
            if (!is_first)
                current++; // only iterate current by one when it is not the
                           // first time

            auto expr = TRY_CALL_RETURN(eval_expression(), {});
            args.push_back(expr);

            if (is_first)
                is_first = false;
        }
        while (match(TokenType::TOKEN_COMMA));
    }

    return args;
}

// e.g. [X, Y, Z, ...]
std::vector<Token> Parser::consume_comma_seperated_token_arguments(TokenType closer) {
    auto args     = std::vector<Token>();
    bool is_first = true;
    if (!match(closer))
    {
        do
        {
            if (!is_first)
                current++; // only iterate current by one when it is not the
                           // first time

            auto token = consume_token();
            args.push_back(*token);

            if (is_first)
                is_first = false;
        }
        while (match(TokenType::TOKEN_COMMA));
    }

    return args;
}

// e.g. (int32, ^char, ...)
std::vector<TypeExpression *> Parser::consume_comma_seperated_types(TokenType closer) {
    auto types    = std::vector<TypeExpression *>();
    bool is_first = true;
    if (!match(closer))
    {
        do
        {
            if (!is_first)
                current++; // only iterate current by one when it is not the
                           // first time

            auto type = TRY_CALL_RETURN(eval_type_expression(), {});
            types.push_back(type);

            if (is_first)
                is_first = false;
        }
        while (match(TokenType::TOKEN_COMMA));
    }

    return types;
}

// e.g. (int x, int y, ...)
std::vector<Tuple<Token, TypeExpression *>> Parser::consume_comma_seperated_params() {
    auto args_types = std::vector<Tuple<Token, TypeExpression *>>();
    bool is_first   = true;
    if (!match(TokenType::TOKEN_PAREN_CLOSE) && !match(TokenType::TOKEN_BRACE_CLOSE))
    {
        do
        {
            if (!is_first)
                current++; // only iterate current by one when it is not the
                           // first time

            auto arg = TRY_CALL_RETURN(consume_token_of_type(TokenType::TOKEN_IDENTIFIER), {});
            TRY_CALL_RETURN(consume_token_of_type(TokenType::TOKEN_COLON), {});
            auto type = TRY_CALL_RETURN(eval_type_expression(), {});

            args_types.push_back({*arg, type});

            if (is_first)
                is_first = false;
        }
        while (match(TokenType::TOKEN_COMMA));
    }

    return args_types;
}

std::vector<Tuple<Token, Expression *>> Parser::consume_comma_seperated_named_arguments(TokenType closer) {
    auto named_args = std::vector<Tuple<Token, Expression *>>();
    bool is_first   = true;
    if (!match(closer))
    {
        do
        {
            if (!is_first)
                current++; // only iterate current by one when it is not the
            // first time

            auto name = TRY_CALL_RETURN(consume_token_of_type(TokenType::TOKEN_IDENTIFIER), {});

            TRY_CALL_RETURN(consume_token_of_type(TokenType::TOKEN_COLON), {});

            auto expression = TRY_CALL_RETURN(eval_expression(), {});
            named_args.push_back({*name, expression});

            if (is_first)
                is_first = false;
        }
        while (match(TokenType::TOKEN_COMMA));
    }

    return named_args;
}