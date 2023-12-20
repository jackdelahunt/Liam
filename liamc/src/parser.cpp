#include "parser.h"

#include <format>
#include <set>
#include <tuple>
#include <utility>

#include "args.h"
#include "ast.h"
#include "baseLayer/debug.h"
#include "errors.h"
#include "liam.h"
#include "token.h"

Parser::Parser(CompilationUnit *compilation_unit) {
    this->compilation_unit = compilation_unit;
    this->current          = 0;
}

void Parser::parse() {
    while (current < this->compilation_unit->token_buffer.size()) {
        auto errors_before = ErrorReporter::error_count();
        auto stmt          = TRY_CALL_VOID(eval_top_level_statement());
        if (ErrorReporter::error_count() > errors_before)
            continue;

        if (stmt->statement_type == StatementType::FN) {
            auto fn_stmt = static_cast<FnStatement *>(stmt);
            compilation_unit->top_level_fn_statements.push_back(fn_stmt);
        } else if (stmt->statement_type == StatementType::STRUCT) {
            auto struct_stmt = static_cast<StructStatement *>(stmt);
            compilation_unit->top_level_struct_statements.push_back(struct_stmt);
        } else if (stmt->statement_type == StatementType::IMPORT) {
            auto import_stmt = static_cast<ImportStatement *>(stmt);
            compilation_unit->top_level_import_statements.push_back(import_stmt);
        } else {
            UNREACHABLE();
        }
    }
}

Statement *Parser::eval_statement() {
    switch (peek()->token_type) {
    case TokenType::TOKEN_LET:
        return eval_let_statement();
        break;
    case TokenType::TOKEN_RETURN:
        return eval_return_statement();
        break;
    case TokenType::TOKEN_BREAK:
        return eval_break_statement();
        break;
    case TokenType::TOKEN_FOR:
        return eval_for_statement();
        break;
    case TokenType::TOKEN_IF:
        return eval_if_statement();
        break;
    case TokenType::TOKEN_CONTINUE:
        return eval_continue_statement();
        break;
    case TokenType::TOKEN_PRINT:
        return eval_print_statement();
        break;
    case TokenType::TOKEN_ASSERT:
        return eval_assert_statement();
        break;
    case TokenType::TOKEN_FN:
    case TokenType::TOKEN_STRUCT:
    case TokenType::TOKEN_IMPORT: {
        ErrorReporter::report_parser_error(
            this->compilation_unit->file_data->absolute_path.string(), peek()->span,
            std::format("unexpected token used to declare new statement in scope '{}'",
                        this->compilation_unit->get_token_string_from_index(consume_token_with_index())));
    } break;
    default:
        return eval_line_starting_expression();
        break;
    }
}

Statement *Parser::eval_top_level_statement() {
    switch (peek()->token_type) {
    case TokenType::TOKEN_FN:
        return eval_fn_statement();
        break;
    case TokenType::TOKEN_STRUCT:
        return eval_struct_statement();
        break;
    case TokenType::TOKEN_IMPORT:
        return eval_import_statement();
        break;
    default: {
        auto token      = consume_token_with_index();
        auto token_data = this->compilation_unit->get_token(token);
        ErrorReporter::report_parser_error(
            this->compilation_unit->file_data->absolute_path.string(), token_data->span,
            std::format("Unexpected token used to declare new statement at top level '{}'",
                        this->compilation_unit->get_token_string_from_index(token)));
        return NULL;
    }
    }
}

LetStatement *Parser::eval_let_statement() {
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_LET));
    TokenIndex identifier = TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_IDENTIFIER));

    TypeExpression *type  = NULL;

    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_COLON));
    if (peek()->token_type != TokenType::TOKEN_ASSIGN) {
        type = TRY_CALL_RET(eval_type_expression());
    }
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_ASSIGN));
    auto expression = TRY_CALL_RET(eval_expression_statement());
    return new LetStatement(identifier, expression->expression, type);
}

ScopeStatement *Parser::eval_scope_statement() {
    auto        statements             = std::vector<Statement *>();
    TokenIndex  open_brace_token_index = TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_BRACE_OPEN));
    Token      *open_brace_token_data  = this->compilation_unit->get_token(open_brace_token_index);
    Option<u64> closing_brace_index =
        find_balance_point(TokenType::TOKEN_BRACE_OPEN, TokenType::TOKEN_BRACE_CLOSE, this->current - 1);

    if (!closing_brace_index.is_some()) {
        ErrorReporter::report_parser_error(this->compilation_unit->file_data->absolute_path.string(),
                                           open_brace_token_data->span, "No closing brace for scope found");
        return NULL;
    }

    while (this->current < closing_brace_index.value()) {
        auto statement = TRY_CALL_RET(eval_statement());
        statements.push_back(statement);
    }
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_BRACE_CLOSE));

    return new ScopeStatement(statements);
}

FnStatement *Parser::eval_fn_statement() {
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_FN));

    auto identifier = TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_IDENTIFIER));

    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_PAREN_OPEN));

    auto params = TRY_CALL_RET(consume_comma_seperated_params());
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_PAREN_CLOSE));

    auto type = TRY_CALL_RET(eval_type_expression());

    auto body = TRY_CALL_RET(eval_scope_statement());
    return new FnStatement(this->compilation_unit, identifier, params, type, body);
}

StructStatement *Parser::eval_struct_statement() {
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_STRUCT));

    auto identifier = TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_IDENTIFIER));

    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_BRACE_OPEN));

    auto member = TRY_CALL_RET(consume_comma_seperated_params());
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_BRACE_CLOSE));
    return new StructStatement(this->compilation_unit, identifier, member, NULL);
}

ReturnStatement *Parser::eval_return_statement() {
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_RETURN));

    Expression *expression = NULL;

    if (peek()->token_type != TokenType::TOKEN_SEMI_COLON) {
        expression = TRY_CALL_RET(eval_expression());
    }

    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_SEMI_COLON));

    return new ReturnStatement(expression);
}

BreakStatement *Parser::eval_break_statement() {
    // might just use an expression statement for this but for now it is a
    // string lit
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_BREAK));
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_SEMI_COLON));
    return new BreakStatement();
}

ForStatement *Parser::eval_for_statement() {
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_FOR));
    TokenIndex value_identifier = TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_IDENTIFIER));
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_COLON));
    Expression     *expression = TRY_CALL_RET(eval_expression());
    ScopeStatement *body       = TRY_CALL_RET(eval_scope_statement());

    // the for type is set later on in the type checking phase
    return new ForStatement(value_identifier, expression, body, ForType::UNDEFINED);
}

IfStatement *Parser::eval_if_statement() {
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_IF));
    auto expression               = TRY_CALL_RET(eval_expression());
    auto body                     = TRY_CALL_RET(eval_scope_statement());

    // next statement might be else so check if the next token is an 'else'
    // if so capture it and own it otherwise just leave the else statement as NULL
    ElseStatement *else_statement = NULL;
    if (peek()->token_type == TokenType::TOKEN_ELSE) {
        else_statement = TRY_CALL_RET(eval_else_statement());
    }

    return new IfStatement(expression, body, else_statement);
}

ElseStatement *Parser::eval_else_statement() {
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_ELSE));

    // check if it is an else if
    if (peek()->token_type == TokenType::TOKEN_IF) {
        auto if_statement = TRY_CALL_RET(eval_if_statement());
        return new ElseStatement(if_statement, NULL);
    }

    auto body = TRY_CALL_RET(eval_scope_statement());
    return new ElseStatement(NULL, body);
}

ExpressionStatement *Parser::eval_expression_statement() {
    auto expression = TRY_CALL_RET(eval_expression());
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_SEMI_COLON));

    return new ExpressionStatement(expression);
}

ContinueStatement *Parser::eval_continue_statement() {
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_CONTINUE));
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_SEMI_COLON));
    return new ContinueStatement();
}

ImportStatement *Parser::eval_import_statement() {
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_IMPORT));
    TokenIndex string_literal = TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_STRING_LITERAL));
    TokenIndex identifier     = TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_IDENTIFIER));
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_SEMI_COLON));
    return new ImportStatement(identifier, string_literal, NULL);
}

PrintStatement *Parser::eval_print_statement() {
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_PRINT));
    Expression *expression = TRY_CALL_RET(eval_expression());
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_SEMI_COLON));
    return new PrintStatement(expression);
}

AssertStatement *Parser::eval_assert_statement() {
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_ASSERT));
    Expression *expression = TRY_CALL_RET(eval_expression());
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_SEMI_COLON));
    return new AssertStatement(expression);
}

Statement *Parser::eval_line_starting_expression() {
    auto lhs = TRY_CALL_RET(eval_expression());

    if (peek()->token_type == TokenType::TOKEN_ASSIGN) {
        TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_ASSIGN));
        auto rhs = TRY_CALL_RET(eval_expression_statement());

        return new AssigmentStatement(lhs, rhs);
    }

    // not assign, after eval expresion only semi colon is left
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_SEMI_COLON));
    return new ExpressionStatement(lhs);
}

/*
 *  === expression precedence === (lower is more precedence)
 *  or
 *  and
 *  == !=
 *  < > >= <=
 *  + -
 *  ?
 *  * / %
 *  @ * !
 *  call() slice[0]
 *  literal (group) new "" null true false []u64{} $0..10
 */

Expression *Parser::eval_expression() {
    return eval_or();
}

Expression *Parser::eval_or() {
    auto expr = TRY_CALL_RET(eval_and());

    while (match(TokenType::TOKEN_OR)) {
        TokenIndex token_index = consume_token_with_index();
        auto       right       = TRY_CALL_RET(eval_and());
        expr = new BinaryExpression(expr, this->compilation_unit->get_token(token_index)->token_type, right);
    }

    return expr;
}

Expression *Parser::eval_and() {
    auto expr = TRY_CALL_RET(eval_equality());

    while (match(TokenType::TOKEN_AND)) {
        TokenIndex token_index = consume_token_with_index();
        auto       right       = TRY_CALL_RET(eval_equality());
        expr = new BinaryExpression(expr, this->compilation_unit->get_token(token_index)->token_type, right);
    }

    return expr;
}

Expression *Parser::eval_equality() {
    auto expr = TRY_CALL_RET(eval_relational());

    while (match(TokenType::TOKEN_NOT_EQUAL) || match(TokenType::TOKEN_EQUAL)) {
        TokenIndex token_index = consume_token_with_index();
        auto       right       = TRY_CALL_RET(eval_relational());
        expr = new BinaryExpression(expr, this->compilation_unit->get_token(token_index)->token_type, right);
    }

    return expr;
}

Expression *Parser::eval_relational() {
    auto expr = TRY_CALL_RET(eval_term());

    while (match(TokenType::TOKEN_LESS) || match(TokenType::TOKEN_GREATER) || match(TokenType::TOKEN_GREATER_EQUAL) ||
           match(TokenType::TOKEN_LESS_EQUAL)) {
        TokenIndex token_index = consume_token_with_index();
        auto       right       = TRY_CALL_RET(eval_term());
        expr = new BinaryExpression(expr, this->compilation_unit->get_token(token_index)->token_type, right);
    }

    return expr;
}

Expression *Parser::eval_term() {
    auto expr = TRY_CALL_RET(eval_factor());

    while (match(TokenType::TOKEN_PLUS) || match(TokenType::TOKEN_MINUS)) {
        TokenIndex token_index = consume_token_with_index();
        auto       right       = TRY_CALL_RET(eval_factor());
        expr = new BinaryExpression(expr, this->compilation_unit->get_token(token_index)->token_type, right);
    }

    return expr;
}

Expression *Parser::eval_factor() {
    auto expr = TRY_CALL_RET(eval_unary());

    while (match(TokenType::TOKEN_STAR) || match(TokenType::TOKEN_SLASH) || match(TokenType::TOKEN_MOD)) {
        TokenIndex token_index = consume_token_with_index();
        auto       right       = TRY_CALL_RET(eval_unary());
        expr = new BinaryExpression(expr, this->compilation_unit->get_token(token_index)->token_type, right);
    }

    return expr;
}

Expression *Parser::eval_unary() {

    if (match(TokenType::TOKEN_AMPERSAND) || match(TokenType::TOKEN_STAR) || match(TokenType::TOKEN_NOT)) {
        TokenIndex token_index = consume_token_with_index();
        auto       expr        = TRY_CALL_RET(eval_unary());

        return new UnaryExpression(expr, this->compilation_unit->get_token(token_index)->token_type);
    }

    return TRY_CALL_RET(eval_postfix());
}

Expression *Parser::eval_postfix() {
    auto expr = TRY_CALL_RET(eval_primary());

    while (true) {
        if (match(TokenType::TOKEN_PAREN_OPEN)) {
            TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_PAREN_OPEN));
            auto call_args = TRY_CALL_RET(consume_comma_seperated_expressions(TokenType::TOKEN_PAREN_CLOSE));
            TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_PAREN_CLOSE));

            expr = new CallExpression(expr, call_args);
        }
        if (match(TokenType::TOKEN_BRACKET_OPEN)) {
            TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_BRACKET_OPEN));
            Expression *subscripter = TRY_CALL_RET(eval_expression());
            TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_BRACKET_CLOSE));

            expr = new SubscriptExpression(expr, subscripter);
        } else if (match(TokenType::TOKEN_DOT)) {
            consume_token_with_index();
            auto identifier = TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_IDENTIFIER));
            expr            = new GetExpression(expr, identifier);
        } else {
            break;
        }
    }

    return expr;
}

Expression *Parser::eval_primary() {
    Token *token = peek();

    switch (token->token_type) {
    case TokenType::TOKEN_NUMBER_LITERAL: {
        return eval_number_literal();
    } break;
    case TokenType::TOKEN_FALSE:
    case TokenType::TOKEN_TRUE: {
        return new BoolLiteralExpression(consume_token_with_index(), token->span);
    } break;
    case TokenType::TOKEN_STRING_LITERAL: {
        return TRY_CALL_RET(eval_string_literal());
    } break;
    case TokenType::TOKEN_IDENTIFIER: {
        return new IdentifierExpression(consume_token_with_index(), token->span);
    } break;
    case TokenType::TOKEN_NEW: {
        return TRY_CALL_RET(eval_struct_instance_expression());
    } break;
    case TokenType::TOKEN_PAREN_OPEN: {
        return TRY_CALL_RET(eval_group_expression());
    } break;
    case TokenType::TOKEN_NULL: {
        return new NullLiteralExpression(consume_token_with_index(), token->span);
    } break;
    case TokenType::TOKEN_ZERO: {
        return new ZeroLiteralExpression(consume_token_with_index(), token->span);
    } break;
    case TokenType::TOKEN_BRACKET_OPEN: {
        return TRY_CALL_RET(eval_static_array_literal());
    } break;
    case TokenType::TOKEN_BRACE_OPEN: {
        return TRY_CALL_RET(eval_range_expression());
    } break;
    default: {
        auto token_index = consume_token_with_index();
        auto token_data  = this->compilation_unit->get_token(token_index);
        ErrorReporter::report_parser_error(
            this->compilation_unit->file_data->absolute_path.string(), token_data->span,
            std::format("Unexpected token '{}' when parsing expression",
                        get_token_type_string(this->compilation_unit->get_token(token_index)->token_type)));
        return NULL;
    }
    }

    UNREACHABLE();
}

Expression *Parser::eval_number_literal() {
    TokenIndex token_index = TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_NUMBER_LITERAL));
    Token     *token       = this->compilation_unit->get_token(token_index);
    return new NumberLiteralExpression(token_index, token->span);
}

Expression *Parser::eval_string_literal() {
    TokenIndex token_index = TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_STRING_LITERAL));
    Token     *token       = this->compilation_unit->get_token(token_index);
    return new StringLiteralExpression(token_index, token->span);
}

Expression *Parser::eval_struct_instance_expression() {
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_NEW));
    TypeExpression *type_expression = TRY_CALL_RET(eval_type_expression());

    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_BRACE_OPEN));
    auto named_expressions = TRY_CALL_RET(consume_comma_seperated_named_arguments(TokenType::TOKEN_BRACE_CLOSE));
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_BRACE_CLOSE));

    return new StructInstanceExpression(type_expression, named_expressions);
}

Expression *Parser::eval_group_expression() {
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_PAREN_OPEN));
    auto expr = TRY_CALL_RET(eval_expression());
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_PAREN_CLOSE));
    return new GroupExpression(expr);
}

Expression *Parser::eval_static_array_literal() {
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_BRACKET_OPEN));
    NumberLiteralExpression *size = (NumberLiteralExpression *)TRY_CALL_RET(eval_number_literal());
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_BRACKET_CLOSE));
    TypeExpression *type_expression = TRY_CALL_RET(eval_type_expression());
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_BRACE_OPEN));
    std::vector<Expression *> expressions =
        TRY_CALL_RET(consume_comma_seperated_expressions(TokenType::TOKEN_BRACE_CLOSE));
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_BRACE_CLOSE));

    return new StaticArrayExpression(size, type_expression, expressions);
}

Expression *Parser::eval_range_expression() {
    // {:}
    // {1:}
    // {:10}

    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_BRACE_OPEN));

    Expression *start = NULL;
    Expression *end   = NULL;

    if (!match(TokenType::TOKEN_COLON)) {
        start = TRY_CALL_RET(eval_expression());
    }

    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_COLON));

    if (!match(TokenType::TOKEN_BRACE_CLOSE)) {
        end = TRY_CALL_RET(eval_expression());
    }

    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_BRACE_CLOSE));

    return new RangeExpression(start, end);
}

/*
 *  === type expression precedence === (lower is more precedence)
 * T
 * ^T, []T
 */
TypeExpression *Parser::eval_type_expression() {
    return eval_type_unary();
}

TypeExpression *Parser::eval_type_unary() {

    // ^
    if (match(TokenType::TOKEN_HAT)) {
        TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_HAT));

        auto type_expression = TRY_CALL_RET(eval_type_unary());
        return new UnaryTypeExpression(UnaryType::POINTER, type_expression);
    }

    // [100], even though static arrays are not unary we can do them here also
    if (match(TokenType::TOKEN_BRACKET_OPEN)) {
        return TRY_CALL_RET(eval_type_staic_or_slice());
    }

    return TRY_CALL_RET(eval_type_postfix());
}

TypeExpression *Parser::eval_type_postfix() {
    auto expr = TRY_CALL_RET(eval_type_primary());

    while (true) {
        if (match(TokenType::TOKEN_DOT)) {
            consume_token_with_index();
            auto identifier = TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_IDENTIFIER));
            expr            = new GetTypeExpression(expr, identifier);
        } else {
            break;
        }
    }

    return expr;
}

TypeExpression *Parser::eval_type_primary() {
    Token *token    = peek();

    auto identifier = TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_IDENTIFIER));
    return new IdentifierTypeExpression(identifier, token->span);
}

TypeExpression *Parser::eval_type_staic_or_slice() {
    TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_BRACKET_OPEN));

    if (!match(TokenType::TOKEN_BRACKET_CLOSE)) { // static array type
        NumberLiteralExpression *expression = (NumberLiteralExpression *)TRY_CALL_RET(eval_number_literal());
        ASSERT_MSG(expression->type == ExpressionType::NUMBER_LITERAL,
                   "assuming all expression from eval_number_literal are number literals");
        TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_BRACKET_CLOSE));

        TypeExpression *type_expression = TRY_CALL_RET(eval_type_unary());
        return new StaticArrayTypeExpression(expression, type_expression);
    } else { // slice type
        TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_BRACKET_CLOSE));
        TypeExpression *type_expression = TRY_CALL_RET(eval_type_unary());
        return new SliceTypeExpression(type_expression);
    }
}

Option<u64> Parser::find_balance_point(TokenType push, TokenType pull, u64 from) {
    u64 current_index = from;
    u64 balance       = 0;

    while (current_index < this->compilation_unit->token_buffer.size()) {
        if (this->compilation_unit->get_token(current_index)->token_type == push) {
            balance++;
            if (balance == 0)
                return Option(current_index);
        }
        if (this->compilation_unit->get_token(current_index)->token_type == pull) {
            balance--;
            if (balance == 0)
                return Option(current_index);
        }

        current_index++;
    }

    return Option<u64>();
}

bool Parser::match(TokenType type) {
    if (this->compilation_unit->token_buffer.size() > 0)
        return peek()->token_type == type;

    return false;
}

Token *Parser::peek(i32 offset) {
    return this->compilation_unit->get_token(current + offset);
}

TokenIndex Parser::consume_token_with_index() {
    if (current >= this->compilation_unit->token_buffer.size())
        panic("No more tokens to consume");

    return this->current++;
}

TokenIndex Parser::consume_token_of_type_with_index(TokenType type) {
    if (this->current >= this->compilation_unit->token_buffer.size()) {
        Token *last_token_data = this->compilation_unit->get_token(this->compilation_unit->token_buffer.size() - 1);
        ErrorReporter::report_parser_error(this->compilation_unit->file_data->absolute_path.string(),
                                           last_token_data->span,
                                           std::format("Expected '{}' but got unexpected end of file",
                                                       get_token_type_string(last_token_data->token_type)));
        return 0;
    }

    TokenIndex current_token_index = this->current++;
    Token     *token               = this->compilation_unit->get_token(current_token_index);
    if (token->token_type != type) {
        ErrorReporter::report_parser_error(this->compilation_unit->file_data->absolute_path.string(), token->span,
                                           std::format("Expected '{}' got '{}'", get_token_type_string(type),
                                                       get_token_type_string(token->token_type)));
        return 0;
    }

    return current_token_index;
}

// e.g. (0, "hello sailor", ...)
std::vector<Expression *> Parser::consume_comma_seperated_expressions(TokenType closer) {
    auto args     = std::vector<Expression *>();
    bool is_first = true;
    if (!match(closer)) {
        do {
            if (!is_first)
                current++; // only iterate current by one when it is not the
                           // first time

            auto expr = TRY_CALL_RET(eval_expression());
            args.push_back(expr);

            if (is_first)
                is_first = false;
        } while (match(TokenType::TOKEN_COMMA));
    }

    return args;
}

// e.g. [X, Y, Z, ...]
std::vector<TokenIndex> Parser::consume_comma_seperated_token_arguments(TokenType closer) {
    auto args     = std::vector<TokenIndex>();
    bool is_first = true;
    if (!match(closer)) {
        do {
            if (!is_first)
                current++; // only iterate current by one when it is not the
                           // first time

            auto token = consume_token_with_index();
            args.push_back(token);

            if (is_first)
                is_first = false;
        } while (match(TokenType::TOKEN_COMMA));
    }

    return args;
}

// e.g. (i32, ^char, ...)
std::vector<TypeExpression *> Parser::consume_comma_seperated_types(TokenType closer) {
    auto types    = std::vector<TypeExpression *>();
    bool is_first = true;
    if (!match(closer)) {
        do {
            if (!is_first)
                current++; // only iterate current by one when it is not the
                           // first time

            auto type = TRY_CALL_RET(eval_type_expression());
            types.push_back(type);

            if (is_first)
                is_first = false;
        } while (match(TokenType::TOKEN_COMMA));
    }

    return types;
}

// e.g. x: T, y: T
CSV Parser::consume_comma_seperated_params() {
    auto args_types = std::vector<std::tuple<TokenIndex, TypeExpression *>>();
    bool is_first   = true;
    if (!match(TokenType::TOKEN_PAREN_CLOSE) && !match(TokenType::TOKEN_BRACE_CLOSE)) {
        do {
            if (!is_first)
                current++; // only iterate current by one when it is not the first time

            auto arg = TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_IDENTIFIER));
            TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_COLON));
            auto type = TRY_CALL_RET(eval_type_expression());

            args_types.emplace_back(arg, type);

            if (is_first)
                is_first = false;
        } while (match(TokenType::TOKEN_COMMA));
    }

    return args_types;
}

std::vector<std::tuple<TokenIndex, Expression *>> Parser::consume_comma_seperated_named_arguments(TokenType closer) {
    auto named_args = std::vector<std::tuple<TokenIndex, Expression *>>();
    bool is_first   = true;
    if (!match(closer)) {
        do {
            if (!is_first)
                this->current++; // only iterate current by one when it is not the
            // first time

            TokenIndex name = TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_IDENTIFIER));

            TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_COLON));

            auto expression = TRY_CALL_RET(eval_expression());
            named_args.emplace_back(name, expression);

            if (is_first)
                is_first = false;
        } while (match(TokenType::TOKEN_COMMA));
    }

    return named_args;
}
