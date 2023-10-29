#include "parser.h"

#include <format>
#include <set>
#include <tuple>
#include <utility>

#include "args.h"
#include "errors.h"
#include "liam.h"
#include "utils.h"

Parser::Parser(CompilationUnit *compilation_unit) {
    this->compilation_unit = compilation_unit;
    this->current          = 0;
}

void Parser::parse() {
    while (current < this->compilation_unit->tokens.size())
    {
        auto errors_before = ErrorReporter::error_count();
        auto stmt          = TRY_CALL(eval_top_level_statement());
        if (ErrorReporter::error_count() > errors_before)
            continue;

        if (stmt->statement_type == StatementType::STATEMENT_FN)
        {
            auto fn_stmt = dynamic_cast<FnStatement *>(stmt);
            compilation_unit->top_level_fn_statements.push_back(fn_stmt);
        }
        else if (stmt->statement_type == StatementType::STATEMENT_STRUCT)
        {
            auto struct_stmt = dynamic_cast<StructStatement *>(stmt);
            compilation_unit->top_level_struct_statements.push_back(struct_stmt);
        }
        else if (stmt->statement_type == StatementType::STATEMENT_IMPORT)
        {
            auto import_stmt = dynamic_cast<ImportStatement *>(stmt);
            compilation_unit->top_level_import_statements.push_back(import_stmt);
        }
        else
        {
            ASSERT_MSG(
                0, "Can only get the above statements from eval_top_level_statment, so this should never trigger"
            );
        }
    }
}

Statement *Parser::eval_statement() {
    switch (peek()->type)
    {
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
    case TokenType::TOKEN_FN:
    case TokenType::TOKEN_STRUCT:
    case TokenType::TOKEN_IMPORT:
    default:
        return eval_line_starting_expression();
        break;
    }
}

Statement *Parser::eval_top_level_statement() {
    switch (peek()->type)
    {
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
        auto token = *consume_token();
        ErrorReporter::report_parser_error(
            this->compilation_unit->file_data->path.string(), token.span,
            std::format("Unexpected token used to declare new statement at top level '{}'", token.string)
        );
        return NULL;
    }
    }
}

LetStatement *Parser::eval_let_statement() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_LET), NULL);
    TokenIndex identifier = TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_IDENTIFIER), NULL);

    TypeExpression *type = NULL;

    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_COLON), NULL);
    if (peek()->type != TokenType::TOKEN_ASSIGN)
    {
        type = TRY_CALL_RET(eval_type_expression(), NULL);
    }
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_ASSIGN), NULL);
    auto expression = TRY_CALL_RET(eval_expression_statement(), NULL);
    return new LetStatement(compilation_unit, identifier, expression->expression, type);
}

ScopeStatement *Parser::eval_scope_statement() {
    auto statements = std::vector<Statement *>();
    auto open_brace = TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACE_OPEN), NULL);
    i32 closing_brace_index =
        find_balance_point(TokenType::TOKEN_BRACE_OPEN, TokenType::TOKEN_BRACE_CLOSE, current - 1);
    // if (closing_brace_index == current + 1)
    // { // if this scope is empty
    //     current++;
    // }
    // else
    if (closing_brace_index < 0)
    {
        ErrorReporter::report_parser_error(
            this->compilation_unit->file_data->path.string(), open_brace->span, "No closing brace for scope found"
        );
        return NULL;
    }

    while (current < closing_brace_index)
    {
        auto statement = TRY_CALL_RET(eval_statement(), NULL);
        statements.push_back(statement);
    }
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACE_CLOSE), NULL);

    return new ScopeStatement(compilation_unit, statements);
}

FnStatement *Parser::eval_fn_statement() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_FN), NULL);

    TypeExpression *parent_type = NULL;

    // check if there is a ( identifier ), if so it means this is a member function
    if (peek()->type == TokenType::TOKEN_PAREN_OPEN)
    {
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_PAREN_OPEN), NULL);
        parent_type = TRY_CALL_RET(eval_type_expression(), NULL);
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_PAREN_CLOSE), NULL);
    }

    auto identifier = TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_IDENTIFIER), NULL);

    auto generics = std::vector<Token>();
    if (peek()->type == TokenType::TOKEN_LESS)
    {
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_LESS), NULL);
        auto types = TRY_CALL_RET(consume_comma_seperated_token_arguments(TokenType::TOKEN_GREATER), NULL);
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_GREATER), NULL);

        generics = types;
    }

    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_PAREN_OPEN), NULL);

    auto params = TRY_CALL_RET(consume_comma_seperated_params(), NULL);
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_PAREN_CLOSE), NULL);

    auto type = TRY_CALL_RET(eval_type_expression(), NULL);

    u8 tag_mask = TRY_CALL_RET(consume_tags(), NULL);

    if (BIT_SET(tag_mask, TAG_EXTERN))
    {
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_SEMI_COLON), NULL);
        return new FnStatement(compilation_unit, parent_type, identifier, generics, params, type, NULL, tag_mask);
    }
    else
    {
        auto body = TRY_CALL_RET(eval_scope_statement(), NULL);
        return new FnStatement(compilation_unit, parent_type, identifier, generics, params, type, body, tag_mask);
    }
}

StructStatement *Parser::eval_struct_statement() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_STRUCT), NULL);

    auto identifier = TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_IDENTIFIER), NULL);

    auto generics = std::vector<Token>();
    if (peek()->type == TokenType::TOKEN_LESS)
    {
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_LESS), NULL);

        auto types = TRY_CALL_RET(consume_comma_seperated_token_arguments(TokenType::TOKEN_GREATER), NULL);
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_GREATER), NULL);
        generics = types;
    }

    u8 tag_mask = TRY_CALL_RET(consume_tags(), NULL);

    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACE_OPEN), NULL);

    auto member = TRY_CALL_RET(consume_comma_seperated_params(), NULL);
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACE_CLOSE), NULL);
    return new StructStatement(compilation_unit, identifier, generics, member, tag_mask);
}

ReturnStatement *Parser::eval_return_statement() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_RETURN), NULL);

    Expression *expression = NULL;

    if (peek()->type != TokenType::TOKEN_SEMI_COLON)
    {
        expression = TRY_CALL_RET(eval_expression(), NULL);
    }

    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_SEMI_COLON), NULL);

    return new ReturnStatement(compilation_unit, expression);
}

BreakStatement *Parser::eval_break_statement() {
    // might just use an expression statement for this but for now it is a
    // string lit
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BREAK), NULL);
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_SEMI_COLON), NULL);
    return new BreakStatement(compilation_unit);
}

ImportStatement *Parser::eval_import_statement() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_IMPORT), NULL);
    auto import_path = dynamic_cast<StringLiteralExpression *>(TRY_CALL_RET(eval_string_literal(), NULL));
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_SEMI_COLON), NULL);

    ASSERT_MSG(import_path != NULL, "Even though it is returning a expression we always assume it is a StringLiteral");

    return new ImportStatement(compilation_unit, import_path);
}

ForStatement *Parser::eval_for_statement() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_FOR), NULL);

    auto assign = TRY_CALL_RET(eval_statement(), NULL);

    auto condition = TRY_CALL_RET(eval_expression(), NULL);

    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_SEMI_COLON), NULL);
    auto update = TRY_CALL_RET(eval_statement(), NULL);

    auto body = TRY_CALL_RET(eval_scope_statement(), NULL);

    return new ForStatement(compilation_unit, assign, condition, update, body);
}

IfStatement *Parser::eval_if_statement() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_IF), NULL);
    auto expression = TRY_CALL_RET(eval_expression(), NULL);
    auto body       = TRY_CALL_RET(eval_scope_statement(), NULL);

    // next statement might be else so check if the next token is an 'else'
    // if so capture it and own it else just leave the else statement as NULL
    ElseStatement *else_statement = NULL;
    if (peek()->type == TokenType::TOKEN_ELSE)
    {
        else_statement = TRY_CALL_RET(eval_else_statement(), NULL);
    }

    return new IfStatement(compilation_unit, expression, body, else_statement);
}

ElseStatement *Parser::eval_else_statement() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_ELSE), NULL);

    // check if it is an else if
    if (peek()->type == TokenType::TOKEN_IF)
    {
        auto if_statement = TRY_CALL_RET(eval_if_statement(), NULL);
        return new ElseStatement(if_statement, NULL);
    }

    auto body = TRY_CALL_RET(eval_scope_statement(), NULL);
    return new ElseStatement(NULL, body);
}

ExpressionStatement *Parser::eval_expression_statement() {
    auto expression = TRY_CALL_RET(eval_expression(), NULL);
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_SEMI_COLON), NULL);

    return new ExpressionStatement(compilation_unit, expression);
}

ContinueStatement *Parser::eval_continue_statement() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_CONTINUE), NULL);
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_SEMI_COLON), NULL);
    return new ContinueStatement(compilation_unit);
}

Statement *Parser::eval_line_starting_expression() {
    auto lhs = TRY_CALL_RET(eval_expression(), NULL);

    if (peek()->type == TokenType::TOKEN_ASSIGN)
    {
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_ASSIGN), NULL);
        auto rhs = TRY_CALL_RET(eval_expression_statement(), NULL);

        return new AssigmentStatement(compilation_unit, lhs, rhs);
    }

    // not assign, after eval expresion only semi colon is left
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_SEMI_COLON), NULL);
    return new ExpressionStatement(compilation_unit, lhs);
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
 *  literal () new "" null true false []u64{}
 */

Expression *Parser::eval_expression() {
    return eval_or();
}

Expression *Parser::eval_or() {
    auto expr = TRY_CALL_RET(eval_and(), NULL);

    while (match(TokenType::TOKEN_OR))
    {
        TokenIndex token_index  = consume_token_with_index();
        auto right = TRY_CALL_RET(eval_and(), NULL);
        expr       = new BinaryExpression(expr, this->compilation_unit->token_buffer[token_index].token_type, right);
    }

    return expr;
}

Expression *Parser::eval_and() {
    auto expr = TRY_CALL_RET(eval_equality(), NULL);

    while (match(TokenType::TOKEN_AND))
    {
        TokenIndex token_index  = consume_token_with_index();
        auto right = TRY_CALL_RET(eval_equality(), NULL);
        expr       = new BinaryExpression(expr, this->compilation_unit->token_buffer[token_index].token_type, right);
    }

    return expr;
}

Expression *Parser::eval_equality() {
    auto expr = TRY_CALL_RET(eval_relational(), NULL);

    while (match(TokenType::TOKEN_NOT_EQUAL) || match(TokenType::TOKEN_EQUAL))
    {
        TokenIndex token_index  = consume_token_with_index();
        auto right = TRY_CALL_RET(eval_relational(), NULL);
        expr       = new BinaryExpression(expr, this->compilation_unit->token_buffer[token_index].token_type, right);
    }

    return expr;
}

Expression *Parser::eval_relational() {
    auto expr = TRY_CALL_RET(eval_term(), NULL);

    while (match(TokenType::TOKEN_LESS) || match(TokenType::TOKEN_GREATER) || match(TokenType::TOKEN_GREATER_EQUAL) ||
           match(TokenType::TOKEN_LESS_EQUAL))
    {
        TokenIndex token_index  = consume_token_with_index();
        auto right = TRY_CALL_RET(eval_term(), NULL);
        expr       = new BinaryExpression(expr, this->compilation_unit->token_buffer[token_index].token_type, right);
    }

    return expr;
}

Expression *Parser::eval_term() {
    auto expr = TRY_CALL_RET(eval_factor(), NULL);

    while (match(TokenType::TOKEN_PLUS) || match(TokenType::TOKEN_MINUS))
    {
        TokenIndex token_index  = consume_token_with_index();
        auto right = TRY_CALL_RET(eval_factor(), NULL);
        expr       = new BinaryExpression(expr, this->compilation_unit->token_buffer[token_index].token_type, right);
    }

    return expr;
}

Expression *Parser::eval_factor() {
    auto expr = TRY_CALL_RET(eval_unary(), NULL);

    while (match(TokenType::TOKEN_STAR) || match(TokenType::TOKEN_SLASH) || match(TokenType::TOKEN_MOD))
    {
        TokenIndex token_index = consume_token_with_index();
        auto right = TRY_CALL_RET(eval_unary(), NULL);
        expr       = new BinaryExpression(expr, this->compilation_unit->token_buffer[token_index].token_type, right);
    }

    return expr;
}

Expression *Parser::eval_unary() {

    if (match(TokenType::TOKEN_AMPERSAND) || match(TokenType::TOKEN_STAR) || match(TokenType::TOKEN_NOT))
    {
        TokenIndex token_index   = consume_token_with_index();
        auto expr = TRY_CALL_RET(eval_unary(), NULL);

        return new UnaryExpression(expr, this->compilation_unit->token_buffer[token_index].token_type);
    }

    return TRY_CALL_RET(eval_postfix(), NULL);
}

Expression *Parser::eval_postfix() {
    auto expr = TRY_CALL_RET(eval_primary(), NULL);

    while (true)
    {
        if (match(TokenType::TOKEN_PAREN_OPEN) || match(TokenType::TOKEN_COLON))
        {
            auto generics = std::vector<TypeExpression *>();
            if (peek()->type == TokenType::TOKEN_COLON)
            {
                TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_COLON), NULL);
                TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_COLON), NULL);
                TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_LESS), NULL);
                auto types = TRY_CALL_RET(consume_comma_seperated_types(TokenType::TOKEN_GREATER), NULL);
                TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_GREATER), NULL);

                generics = types;
            }

            TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_PAREN_OPEN), NULL);
            auto call_args = TRY_CALL_RET(consume_comma_seperated_arguments(TokenType::TOKEN_PAREN_CLOSE), NULL);
            TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_PAREN_CLOSE), NULL);

            expr = new CallExpression(expr, call_args, generics);
        }
        else if (match(TokenType::TOKEN_DOT))
        {
            consume_token();
            auto identifier = TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_IDENTIFIER), NULL);
            expr            = new GetExpression(expr, *identifier);
        }
        else if (match(TokenType::TOKEN_BRACKET_OPEN))
        {
            TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACKET_OPEN), NULL);
            auto subscript_by = TRY_CALL_RET(eval_postfix(), NULL);
            TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACKET_CLOSE), NULL);

            expr = new SubscriptExpression(expr, subscript_by);
        }
        else
        {
            break;
        }
    }

    return expr;
}

Expression *Parser::eval_primary() {
    auto type = peek()->type;

    if (type == TokenType::TOKEN_NUMBER_LITERAL)
        return new NumberLiteralExpression(consume_token_with_index());
    else if (type == TokenType::TOKEN_STRING_LITERAL)
        return TRY_CALL_RET(eval_string_literal(), NULL);
    else if (type == TokenType::TOKEN_TRUE || type == TokenType::TOKEN_FALSE)
        return new BoolLiteralExpression(consume_token_with_index());
    else if (type == TokenType::TOKEN_IDENTIFIER)
        return new IdentifierExpression(consume_token_with_index());
    else if (type == TokenType::TOKEN_NEW)
        return TRY_CALL_RET(eval_instantiate_expression(), NULL);
    else if (type == TokenType::TOKEN_PAREN_OPEN)
        return eval_group_expression();
    else if (type == TokenType::TOKEN_NULL)
        return new NullLiteralExpression(*consume_token());
    else if (type == TokenType::TOKEN_ZERO)
        return new ZeroLiteralExpression(*consume_token());
    else if (type == TokenType::TOKEN_FN)
        return TRY_CALL_RET(eval_fn(), NULL);
    else if (type == TokenType::TOKEN_BRACKET_OPEN)
        return TRY_CALL_RET(eval_slice_literal(), NULL);
    else
    {
        auto token = *consume_token();
        ErrorReporter::report_parser_error(
            this->compilation_unit->file_data->path.string(), token.span,
            std::format("Unexpected token when parsing expression '{}'", get_token_type_string(token.type))
        );
        return NULL;
    }

    return new Expression(); // empty expression found -- like when a
                             // return has no expression
}

Expression *Parser::eval_string_literal() {
    auto literal = TRY_CALL_RET(consume_token_of_type_with_index(TokenType::TOKEN_STRING_LITERAL), NULL);
    return new StringLiteralExpression(literal);
}

Expression *Parser::eval_fn() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_FN), NULL);

    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_PAREN_OPEN), NULL);
    auto params = TRY_CALL_RET(consume_comma_seperated_params(), NULL);
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_PAREN_CLOSE), NULL);

    auto type = TRY_CALL_RET(eval_type_expression(), NULL);

    auto body = TRY_CALL_RET(eval_scope_statement(), NULL);
    return new FnExpression(params, type, body);
}

Expression *Parser::eval_slice_literal() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACKET_OPEN), NULL);
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACKET_CLOSE), NULL);

    auto type = TRY_CALL_RET(eval_type_expression(), NULL);

    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACE_OPEN), NULL);
    auto expressions = TRY_CALL_RET(consume_comma_seperated_arguments(TokenType::TOKEN_BRACE_CLOSE), NULL);
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACE_CLOSE), NULL);

    return new SliceLiteralExpression(type, expressions);
}

Expression *Parser::eval_instantiate_expression() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_NEW), NULL);
    Expression *expression = TRY_CALL_RET(eval_struct_instance_expression(), NULL);
    return new InstantiateExpression(expression);
}

Expression *Parser::eval_struct_instance_expression() {
    auto identifier = TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_IDENTIFIER), NULL);

    auto generics = std::vector<TypeExpression *>();
    if (peek()->type == TokenType::TOKEN_LESS)
    {
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_LESS), NULL);
        auto types = TRY_CALL_RET(consume_comma_seperated_types(TokenType::TOKEN_GREATER), NULL);
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_GREATER), NULL);

        generics = types;
    }

    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACE_OPEN), NULL);
    auto named_expressions = TRY_CALL_RET(consume_comma_seperated_named_arguments(TokenType::TOKEN_BRACE_CLOSE), NULL);
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_BRACE_CLOSE), NULL);

    return new StructInstanceExpression(*identifier, generics, named_expressions);
}

Expression *Parser::eval_group_expression() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_PAREN_OPEN), NULL);
    auto expr = TRY_CALL_RET(eval_expression(), NULL);
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_PAREN_CLOSE), NULL);
    return new GroupExpression(expr);
}

/*
 *  === type expression precedence === (lower is more precedence)
 * |
 * ^
 * identifier
 */
TypeExpression *Parser::eval_type_expression() {
    return eval_type_unary();
}

TypeExpression *Parser::eval_type_unary() {

    // ^..
    if (match(TokenType::TOKEN_HAT) && peek(1)->type == TokenType::TOKEN_DOT && peek(2)->type == TokenType::TOKEN_DOT)
    {
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_HAT), NULL);
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_DOT), NULL);
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_DOT), NULL);

        auto type_expression = TRY_CALL_RET(eval_type_unary(), NULL);
        return new UnaryTypeExpression(UnaryType::POINTER_SLICE, type_expression);
    }

    // ^
    if (match(TokenType::TOKEN_HAT))
    {
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_HAT), NULL);

        auto type_expression = TRY_CALL_RET(eval_type_unary(), NULL);
        return new UnaryTypeExpression(UnaryType::POINTER, type_expression);
    }

    return eval_type_specified_generics();
}

TypeExpression *Parser::eval_type_specified_generics() {
    auto primary = TRY_CALL_RET(eval_type_primary(), NULL);

    if (match(TokenType::TOKEN_LESS))
    {
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_LESS), NULL);
        auto generics = TRY_CALL_RET(consume_comma_seperated_types(TokenType::TOKEN_GREATER), NULL);
        TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_GREATER), NULL);

        return new SpecifiedGenericsTypeExpression(primary, generics);
    }

    return primary;
}

TypeExpression *Parser::eval_type_primary() {

    if (peek()->type == TokenType::TOKEN_FN)
    {
        return TRY_CALL_RET(eval_type_fn(), NULL);
    }

    auto identifier = TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_IDENTIFIER), NULL);
    return new IdentifierTypeExpression(*identifier);
}

TypeExpression *Parser::eval_type_fn() {
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_FN), NULL);
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_PAREN_OPEN), NULL);
    auto params = TRY_CALL_RET(consume_comma_seperated_types(TokenType::TOKEN_PAREN_CLOSE), NULL);
    TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_PAREN_CLOSE), NULL);

    auto return_type = TRY_CALL_RET(eval_type_expression(), NULL);

    return new FnTypeExpression(params, return_type);
}

i32 Parser::find_balance_point(TokenType push, TokenType pull, i32 from) {
    i32 current_index = from;
    i32 balance       = 0;

    while (current_index < this->compilation_unit->tokens.size())
    {
        if (this->compilation_unit->tokens.at(current_index).type == push)
        {
            balance++;
            if (balance == 0)
                return current_index;
        }
        if (this->compilation_unit->tokens.at(current_index).type == pull)
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
    if (this->compilation_unit->tokens.size() > 0)
        return peek()->type == type;

    return false;
}

Token *Parser::peek(i32 offset) {
    return &this->compilation_unit->tokens.at(current + offset);
}

Token *Parser::consume_token() {
    if (current >= this->compilation_unit->tokens.size())
        panic("No more tokens to consume");

    return &this->compilation_unit->tokens.at(current++);
}

TokenIndex Parser::consume_token_with_index() {
    if (current >= this->compilation_unit->tokens.size())
        panic("No more tokens to consume");

    return this->current++;
}

Token *Parser::consume_token_of_type(TokenType type) {
    if (current >= this->compilation_unit->tokens.size())
    {
        auto last_token = this->compilation_unit->tokens.at(this->compilation_unit->tokens.size() - 1);
        ErrorReporter::report_parser_error(
            this->compilation_unit->file_data->path.string(), last_token.span,
            std::format("Expected '{}' but got unexpected end of file", last_token.string)
        );
        return NULL;
    }

    auto t_ptr = &this->compilation_unit->tokens.at(current++);
    if (t_ptr->type != type)
    {
        ErrorReporter::report_parser_error(
            this->compilation_unit->file_data->path.string(), t_ptr->span,
            std::format("Expected '{}' got '{}'", get_token_type_string(type), t_ptr->string)
        );
        return NULL;
    }

    return t_ptr;
}

TokenIndex Parser::consume_token_of_type_with_index(TokenType type) {
    if (this->current >= this->compilation_unit->tokens.size())
    {
        TokenData last_token_data =
            this->compilation_unit->token_buffer.at(this->compilation_unit->token_buffer.size() - 1);
        ErrorReporter::report_parser_error(
            this->compilation_unit->file_data->path.string(), last_token_data.span,
            std::format("Expected '{}' but got unexpected end of file", "TODO: add token string HERE!!")
        );
        return 0;
    }

    TokenIndex current_token_index = this->current++;
    TokenData *token_data_ptr      = &this->compilation_unit->token_buffer[current_token_index];
    if (token_data_ptr->token_type != type)
    {
        ErrorReporter::report_parser_error(
            this->compilation_unit->file_data->path.string(), token_data_ptr->span,
            std::format("Expected '{}' got '{}'", get_token_type_string(type), "TODO: add token string HERE!!")
        );
        return 0;
    }

    return current_token_index;
}

TokenData *Parser::get_token_data(TokenIndex token_index) {
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

            auto expr = TRY_CALL_RET(eval_expression(), {});
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

// e.g. (i32, ^char, ...)
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

            auto type = TRY_CALL_RET(eval_type_expression(), {});
            types.push_back(type);

            if (is_first)
                is_first = false;
        }
        while (match(TokenType::TOKEN_COMMA));
    }

    return types;
}

// e.g. (int x, int y, ...)
CSV Parser::consume_comma_seperated_params() {
    auto args_types = std::vector<std::tuple<Token, TypeExpression *>>();
    bool is_first   = true;
    if (!match(TokenType::TOKEN_PAREN_CLOSE) && !match(TokenType::TOKEN_BRACE_CLOSE))
    {
        do
        {
            if (!is_first)
                current++; // only iterate current by one when it is not the
                           // first time

            auto arg = TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_IDENTIFIER), {});
            TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_COLON), {});
            auto type = TRY_CALL_RET(eval_type_expression(), {});

            args_types.emplace_back(*arg, type);

            if (is_first)
                is_first = false;
        }
        while (match(TokenType::TOKEN_COMMA));
    }

    return args_types;
}

std::vector<std::tuple<Token, Expression *>> Parser::consume_comma_seperated_named_arguments(TokenType closer) {
    auto named_args = std::vector<std::tuple<Token, Expression *>>();
    bool is_first   = true;
    if (!match(closer))
    {
        do
        {
            if (!is_first)
                this->current++; // only iterate current by one when it is not the
            // first time

            auto name = TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_IDENTIFIER), {});

            TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_COLON), {});

            auto expression = TRY_CALL_RET(eval_expression(), {});
            named_args.emplace_back(*name, expression);

            if (is_first)
                is_first = false;
        }
        while (match(TokenType::TOKEN_COMMA));
    }

    return named_args;
}

// @extern @private
u8 Parser::consume_tags() {
    u8 flag_mask = 0;

    // consume possible tage
    while (peek()->type == TokenType::TOKEN_TAG)
    {
        Token tag = *TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_TAG), NULL);

        if (compare_string(tag.string, "@extern"))
        {
            if (BIT_SET(flag_mask, TAG_EXTERN))
            {
                goto duplicate_error;
            }

            SET_BIT(flag_mask, TAG_EXTERN);
            continue;
        }
        else if (compare_string(tag.string, "@private"))
        {
            if (BIT_SET(flag_mask, TAG_PRIVATE))
            {
                goto duplicate_error;
            }

            SET_BIT(flag_mask, TAG_PRIVATE);
            continue;
        }
        else
        {
            ErrorReporter::report_parser_error(
                this->compilation_unit->file_data->path.string(), tag.span, "Unknown tag give \"" + tag.string + "\""
            );
            return 0;
        }

    duplicate_error:
        ErrorReporter::report_parser_error(
            this->compilation_unit->file_data->path.string(), tag.span, "Duplicate tag given for \"" + tag.string + "\""
        );
        return true;
    }

    return flag_mask;
}
