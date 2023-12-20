#include "type_checker.h"

#include <assert.h>
#include <format>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "ast.h"
#include "baseLayer/debug.h"
#include "compilation_unit.h"
#include "errors.h"
#include "liam.h"
#include "utils.h"

TypeChecker::TypeChecker() {
    this->compilation_unit   = NULL;
    this->compilation_bundle = NULL;
    this->scopes             = std::list<Scope>();
}

void TypeChecker::new_scope() {
    this->scopes.push_back(Scope());
}

void TypeChecker::delete_scope() {
    ASSERT_MSG(this->scopes.size() > 0, "Must be an active scope to add to");
    this->scopes.pop_back();
}

void TypeChecker::add_to_scope(TokenIndex token_index, TypeInfo *type_info) {
    ASSERT_MSG(this->scopes.size() > 0, "Must be an active scope to add to");
    std::string identifier           = this->compilation_unit->get_token_string_from_index(token_index);
    this->scopes.front()[identifier] = type_info;
}

TypeInfo *TypeChecker::get_from_scope(TokenIndex token_index) {
    ASSERT_MSG(this->scopes.size() > 0, "Must be an active scope to add to");
    std::string identifier = this->compilation_unit->get_token_string_from_index(token_index);

    // look into all of the local scopes going up the chain
    for (auto iter = this->scopes.rbegin(); iter != this->scopes.rend(); iter++) {
        if (iter->count(identifier) > 0) {
            return (*iter)[identifier];
        }
    }

    // if there is nothing then check the fn scope
    TypeInfo *type_info = this->compilation_unit->get_fn_from_scope(token_index);
    if (type_info != NULL) {
        return type_info;
    }

    return this->compilation_unit->get_namespace_from_scope(token_index);
}

void TypeChecker::type_check(CompilationBundle *bundle) {
    this->compilation_bundle = bundle;
    for (CompilationUnit *cu : bundle->compilation_units) {
        this->compilation_unit = cu;

        // add symbols for structs and fns
        for (auto stmt : this->compilation_unit->top_level_import_statements) {
            ASSERT(stmt->statement_type != StatementType::UNDEFINED);
            TRY_CALL_VOID(type_check_import_statement(stmt));
        }
    }

    for (CompilationUnit *cu : bundle->compilation_units) {
        this->compilation_unit = cu;

        // add symbols for structs and fns
        for (auto stmt : this->compilation_unit->top_level_struct_statements) {
            ASSERT(stmt->statement_type != StatementType::UNDEFINED);
            TRY_CALL_VOID(type_check_struct_symbol(stmt));
        }

        for (auto stmt : this->compilation_unit->top_level_fn_statements) {
            ASSERT(stmt->statement_type != StatementType::UNDEFINED);
            TRY_CALL_VOID(type_check_fn_symbol(stmt));
        }
    }

    for (CompilationUnit *cu : bundle->compilation_units) {
        this->compilation_unit = cu;

        // type the body of the structs and the declarations of the fns
        for (auto stmt : this->compilation_unit->top_level_struct_statements) {
            ASSERT(stmt->statement_type != StatementType::UNDEFINED);
            TRY_CALL_VOID(type_check_struct_statement_full(stmt));
        }

        for (auto stmt : this->compilation_unit->top_level_fn_statements) {
            ASSERT(stmt->statement_type != StatementType::UNDEFINED);
            TRY_CALL_VOID(type_check_fn_decl(stmt));
        }
    }

    // type check struct bodies looking for recursive types
    // and then use a topological sort
    std::vector<StructStatement *> all_struct_statements;
    for (CompilationUnit *cu : bundle->compilation_units) {
        for (StructStatement *stmt : cu->top_level_struct_statements) {
            all_struct_statements.push_back(stmt);
        }
    }

    this->compilation_bundle->sorted_types = TRY_CALL_VOID(topilogical_sort(all_struct_statements));

    for (CompilationUnit *cu : bundle->compilation_units) {
        this->compilation_unit = cu;

        // finally do the function body pass
        for (auto stmt : this->compilation_unit->top_level_fn_statements) {
            TRY_CALL_VOID(type_check_fn_statement_full(stmt));
        }
    }

    TRY_CALL_VOID(find_entry_point());
}

void TypeChecker::find_entry_point() {
    for (CompilationUnit *cu : this->compilation_bundle->compilation_units) {
        for (auto stmt : cu->top_level_fn_statements) {
            if (cu->get_token_string_from_index(stmt->identifier) == "main") {
                this->compilation_bundle->entry_point = stmt;
                return;
            }
        }
    }

    TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
        .set_message("no enrty point found, 'main' function must be defined")
        .report();
}

void TypeChecker::type_check_import_statement(ImportStatement *statement) {
    // trimming the " from either side of the token, this is becase it is a string literal
    // and that means
    std::string import_path = this->compilation_unit->get_token_string_from_index(statement->string_literal);
    trim(import_path, "\"");

    std::filesystem::path this_compilation_unit_parent_dir_path =
        this->compilation_unit->file_data->absolute_path.parent_path();
    Option<u64> compilation_unit_index = this->compilation_bundle->get_compilation_unit_index_with_path_relative_from(
        this_compilation_unit_parent_dir_path.string(), import_path);

    if (!compilation_unit_index.is_some()) {
        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message(std::format("cannot find file with import path '{}' ", import_path))
            .report();

        return;
    }

    NamespaceTypeInfo *type_info = new NamespaceTypeInfo(compilation_unit_index.value());

    ScopeActionStatus status     = this->compilation_unit->add_namespace_to_scope(statement->identifier, type_info);

    if (status == ScopeActionStatus::ALREADY_EXISTS) {
        std::string identifier = this->compilation_unit->get_token_string_from_index(statement->identifier);
        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message(std::format("duplicate creation of namespace identifier '{}' ", identifier))
            .report();

        return;
    }

    statement->namespace_type_info = type_info;
}

void TypeChecker::type_check_fn_symbol(FnStatement *statement) {
    ScopeActionStatus status = this->compilation_unit->add_fn_to_scope(statement->identifier, new FnTypeInfo(NULL, {}));
    if (status == ScopeActionStatus::ALREADY_EXISTS) {
        std::string identifier = this->compilation_unit->get_token_string_from_index(statement->identifier);
        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message(std::format("duplicate creation of fn '{}' ", identifier))
            .report();

        return;
    }
}

void TypeChecker::type_check_struct_symbol(StructStatement *statement) {
    // we cannot be sure if we type check all of the struct that all members
    // will be resolved. As structs after this one might be referenced
    // add it to the table and leave its type info blank until we type check it

    ScopeActionStatus status =
        this->compilation_unit->add_type_to_scope(statement->identifier, new StructTypeInfo(statement, {}));
    if (status == ScopeActionStatus::ALREADY_EXISTS) {
        std::string identifier = this->compilation_unit->get_token_string_from_index(statement->identifier);
        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message(std::format("duplicate creation of type '{}' ", identifier))
            .report();

        return;
    }
}

void TypeChecker::type_check_fn_decl(FnStatement *statement) {
    auto param_type_infos = std::vector<TypeInfo *>();
    for (auto &[identifier, expr] : statement->params) {
        TRY_CALL_VOID(type_check_type_expression(expr))
        param_type_infos.push_back(expr->type_info);
    }

    TRY_CALL_VOID(type_check_type_expression(statement->return_type));

    std::string name_as_string = this->compilation_unit->get_token_string_from_index(statement->identifier);

    // the current scope is always the fn scope
    auto current_type_info     = (FnTypeInfo *)this->compilation_unit->get_fn_from_scope(statement->identifier);
    ASSERT(current_type_info);
    current_type_info->return_type = statement->return_type->type_info;
    current_type_info->args        = param_type_infos;
}

void TypeChecker::type_check_fn_statement_full(FnStatement *statement) {
    // getting the already existing type info for the function is done in 2 ways
    // 1. looking into the symbol table of the current scope (normal function)
    // 2. looking into the member functions of a type (member function)
    FnTypeInfo *fn_type_info = NULL;

    fn_type_info             = (FnTypeInfo *)this->compilation_unit->get_fn_from_scope(statement->identifier);
    ASSERT(fn_type_info);

    // params and get type expressions
    auto args = std::vector<std::tuple<TokenIndex, TypeInfo *>>();
    args.reserve(fn_type_info->args.size());
    args.resize(fn_type_info->args.size());
    for (u64 i = 0; i < fn_type_info->args.size(); i++) {
        auto &[identifier, expr] = statement->params.at(i);
        args[i]                  = {identifier, fn_type_info->args.at(i)};
    }

    this->new_scope();
    for (auto &[token_index, type_info] : args) {
        this->add_to_scope(token_index, type_info);
    }
    TRY_CALL_VOID(type_check_scope_statement(statement->body));
    this->delete_scope();

    for (auto stmt : statement->body->statements) {
        if (stmt->statement_type == StatementType::RETURN) {
            auto rt = static_cast<ReturnStatement *>(stmt);

            if (fn_type_info->return_type->type == TypeInfoType::VOID) {
                if (rt->expression != NULL) {
                    TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
                        .set_expr_1(rt->expression)
                        .set_message("found expression in return when return type is void")
                        .report();

                    return;
                }
            } else {
                if (!type_match(fn_type_info->return_type, rt->expression->type_info)) {
                    ErrorReporter::report_type_checker_error(compilation_unit->file_data->absolute_path.string(),
                                                             rt->expression, NULL, statement->return_type, NULL,
                                                             "mismatch types in function, return types do not match");
                    return;
                }
            }
        }
    }
}

void TypeChecker::type_check_struct_statement_full(StructStatement *statement) {
    auto members_type_info = std::vector<std::tuple<std::string, TypeInfo *>>();
    members_type_info.reserve(statement->members.size());
    members_type_info.resize(statement->members.size());
    for (u64 i = 0; i < statement->members.size(); i++) {
        auto [member, expr] = statement->members.at(i);
        TRY_CALL_VOID(type_check_type_expression(expr));
        std::string member_string = this->compilation_unit->get_token_string_from_index(member);
        members_type_info[i]      = {member_string, expr->type_info};
    }

    StructTypeInfo *struct_type_info =
        (StructTypeInfo *)this->compilation_unit->get_type_from_scope(statement->identifier);
    ASSERT(struct_type_info != NULL);
    struct_type_info->members = members_type_info;

    statement->type_info      = struct_type_info;
}

void TypeChecker::type_check_statement(Statement *statement) {
    // TODO: why are these void functions returning??
    switch (statement->statement_type) {
    case StatementType::RETURN:
        return type_check_return_statement(static_cast<ReturnStatement *>(statement));
    case StatementType::BREAK:
        return type_check_break_statement(static_cast<BreakStatement *>(statement));
    case StatementType::ASSIGNMENT:
        return type_check_assigment_statement(static_cast<AssigmentStatement *>(statement));
    case StatementType::EXPRESSION:
        return type_check_expression_statement(static_cast<ExpressionStatement *>(statement));
    case StatementType::LET:
        return type_check_let_statement(static_cast<LetStatement *>(statement));
    case StatementType::FOR:
        return type_check_for_statement(static_cast<ForStatement *>(statement));
    case StatementType::IF:
        return type_check_if_statement(static_cast<IfStatement *>(statement));
    case StatementType::PRINT:
        return type_check_print_statement(static_cast<PrintStatement *>(statement));
    case StatementType::ASSERT:
        return type_check_assert_statement(static_cast<AssertStatement *>(statement));
    case StatementType::WHILE:
        return type_check_while_statement(static_cast<WhileStatement *>(statement));
    case StatementType::CONTINUE:
        break;
    case StatementType::STRUCT:
    case StatementType::FN:
    case StatementType::IMPORT:
        ASSERT_MSG(0, "These should of already been type checked in the first pass");
    case StatementType::UNDEFINED:
        UNREACHABLE();
    default:
        UNREACHABLE();
    }
}

void TypeChecker::type_check_return_statement(ReturnStatement *statement) {
    if (statement->expression)
        TRY_CALL_VOID(type_check_expression(statement->expression));
}

void TypeChecker::type_check_break_statement(BreakStatement *statement) {
}

void TypeChecker::type_check_let_statement(LetStatement *statement) {
    TRY_CALL_VOID(type_check_expression(statement->rhs));

    // if let type is there type match both and set var type
    // to the let type... else just set it to the rhs
    if (statement->type != NULL) {
        TRY_CALL_VOID(type_check_type_expression(statement->type));
        if (!type_match(statement->type->type_info, statement->rhs->type_info)) {
            ErrorReporter::report_type_checker_error(compilation_unit->file_data->absolute_path.string(),
                                                     statement->rhs, NULL, statement->type, NULL,
                                                     "mismatched types in let statement");
            return;
        }
    } else if (statement->rhs->type_info->type == TypeInfoType::ANY) {
        ErrorReporter::report_type_checker_error(compilation_unit->file_data->absolute_path.string(), statement->rhs,
                                                 NULL, NULL, NULL, "cannot infer type of expression in let statement");
        return;
    }

    if (statement->type != NULL) {
        this->add_to_scope(statement->identifier, statement->type->type_info);
    } else {
        this->add_to_scope(statement->identifier, statement->rhs->type_info);
    }
}

void TypeChecker::type_check_scope_statement(ScopeStatement *statement) {
    for (auto stmt : statement->statements) {
        TRY_CALL_VOID(type_check_statement(stmt));
    }
}

void TypeChecker::type_check_for_statement(ForStatement *statement) {
    TRY_CALL_VOID(type_check_expression(statement->expression));

    TypeInfoType expression_type_info_type = statement->expression->type_info->type;

    if (expression_type_info_type == TypeInfoType::STATIC_ARRAY) {
        statement->for_type = ForType::STATIC_ARRAY;
    } else if (expression_type_info_type == TypeInfoType::SLICE) {
        statement->for_type = ForType::SLICE;
    } else if (expression_type_info_type == TypeInfoType::RANGE) {
        statement->for_type = ForType::RANGE;
    } else {
        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message("incorrect type given in for statement, must use a static array, slice or range")
            .set_expr_1(statement->expression)
            .report();
        return;
    }

    ASSERT(statement->for_type != ForType::UNDEFINED);

    TypeInfo *value_type_info = NULL;

    switch (statement->for_type) {
    case ForType::STATIC_ARRAY: {
        value_type_info = ((StaticArrayTypeInfo *)statement->expression->type_info)->base_type;
    } break;
    case ForType::SLICE: {
        value_type_info = ((SliceTypeInfo *)statement->expression->type_info)->base_type;
    } break;
    case ForType::RANGE: {
        RangeExpression *range_expression = (RangeExpression *)statement->expression;
        if (range_expression->start == NULL || range_expression->end == NULL) {
            TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
                .set_message("range expression must have both a start and end value in for loops")
                .set_expr_1(statement->expression)
                .report();
            return;
        }

        value_type_info = range_expression->start->type_info;
    } break;
    default:
        UNREACHABLE();
    }

    this->new_scope();
    this->add_to_scope(statement->value_identifier, value_type_info);
    TRY_CALL_VOID(type_check_scope_statement(statement->body));
    this->delete_scope();
}

void TypeChecker::type_check_if_statement(IfStatement *statement) {
    TRY_CALL_VOID(type_check_expression(statement->expression));

    if (!type_match(statement->expression->type_info, this->compilation_unit->global_type_scope["bool"])) {
        ErrorReporter::report_type_checker_error(compilation_unit->file_data->absolute_path.string(),
                                                 statement->expression, NULL, NULL, NULL,
                                                 "can only pass boolean expressions to if statements");
        return;
    }

    this->new_scope();
    TRY_CALL_VOID(type_check_scope_statement(statement->body));
    this->delete_scope();

    if (statement->else_statement) {
        // might not have else statement
        // not using symbol table copy either as symbols
        // should not leak to sub statements
        TRY_CALL_VOID(type_check_else_statement(statement->else_statement));
    }
}

void TypeChecker::type_check_else_statement(ElseStatement *statement) {
    if (statement->if_statement) {
        TRY_CALL_VOID(type_check_if_statement(statement->if_statement));
    }

    if (statement->body) {
        TRY_CALL_VOID(type_check_scope_statement(statement->body));
    }
}

void TypeChecker::type_check_assigment_statement(AssigmentStatement *statement) {
    TRY_CALL_VOID(type_check_expression(statement->lhs));

    if (statement->lhs->category != ExpressionCategory::LVALUE) {
        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message("trying to assign to a rvalue, only lvalues can be assigned")
            .set_expr_1(statement->lhs)
            .report();
        return;
    }

    TRY_CALL_VOID(type_check_expression(statement->assigned_to->expression));

    if (!type_match(statement->lhs->type_info, statement->assigned_to->expression->type_info)) {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->absolute_path.string(), statement->lhs, statement->assigned_to->expression,
            NULL, NULL, "type mismatch, trying to assign a identifier to an expression of different type");
        return;
    }
}

void TypeChecker::type_check_expression_statement(ExpressionStatement *statement) {
    TRY_CALL_VOID(type_check_expression(statement->expression));
}

void TypeChecker::type_check_print_statement(PrintStatement *statement) {
    TRY_CALL_VOID(type_check_expression(statement->expression));
}

void TypeChecker::type_check_assert_statement(AssertStatement *statement) {
    TRY_CALL_VOID(type_check_expression(statement->expression));
    if (statement->expression->type_info->type != TypeInfoType::BOOLEAN) {
        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message("assert statement must be a boolean expression")
            .set_expr_1(statement->expression)
            .report();
    }
}

void TypeChecker::type_check_while_statement(WhileStatement *statement) {
    TRY_CALL_VOID(type_check_expression(statement->expression));
    if (statement->expression->type_info->type != TypeInfoType::BOOLEAN) {
        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message("condition for while statement must be a boolean")
            .set_expr_1(statement->expression)
            .report();
    }

    this->new_scope();
    TRY_CALL_VOID(type_check_scope_statement(statement->body));
    this->delete_scope();
}

void TypeChecker::type_check_expression(Expression *expression) {
    switch (expression->type) {
    case ExpressionType::STRING_LITERAL:
        return type_check_string_literal_expression(static_cast<StringLiteralExpression *>(expression));
        break;
    case ExpressionType::NUMBER_LITERAL:
        return type_check_number_literal_expression(static_cast<NumberLiteralExpression *>(expression));
        break;
    case ExpressionType::BOOL_LITERAL:
        return type_check_bool_literal_expression(static_cast<BoolLiteralExpression *>(expression));
        break;
    case ExpressionType::CALL:
        return type_check_call_expression(static_cast<CallExpression *>(expression));
        break;
    case ExpressionType::IDENTIFIER:
        return type_check_identifier_expression(static_cast<IdentifierExpression *>(expression));
        break;
    case ExpressionType::BINARY:
        return type_check_binary_expression(static_cast<BinaryExpression *>(expression));
        break;
    case ExpressionType::UNARY:
        return type_check_unary_expression(static_cast<UnaryExpression *>(expression));
        break;
    case ExpressionType::GET:
        return type_check_get_expression(static_cast<GetExpression *>(expression));
        break;
    case ExpressionType::GROUP:
        return type_check_group_expression(static_cast<GroupExpression *>(expression));
        break;
    case ExpressionType::NULL_LITERAL:
        return type_check_null_literal_expression(static_cast<NullLiteralExpression *>(expression));
        break;
    case ExpressionType::ZERO_LITERAL:
        return type_check_zero_literal_expression(static_cast<ZeroLiteralExpression *>(expression));
        break;
    case ExpressionType::STRUCT_INSTANCE:
        return type_check_struct_instance_expression(static_cast<StructInstanceExpression *>(expression));
        break;
    case ExpressionType::STATIC_ARRAY:
        return type_check_static_array_literal_expression(static_cast<StaticArrayExpression *>(expression));
        break;
    case ExpressionType::SUBSCRIPT:
        return type_check_subscript_expression(static_cast<SubscriptExpression *>(expression));
        break;
    case ExpressionType::RANGE:
        return type_check_range_expression(static_cast<RangeExpression *>(expression));
        break;
    case ExpressionType::UNDEFINED:
        UNREACHABLE();
    default:
        UNREACHABLE();
    }
}

void TypeChecker::type_check_binary_expression(BinaryExpression *expression) {
    TRY_CALL_VOID(type_check_expression(expression->left));
    TRY_CALL_VOID(type_check_expression(expression->right));

    if (!type_match(expression->left->type_info, expression->right->type_info)) {
        ErrorReporter::report_type_checker_error(compilation_unit->file_data->absolute_path.string(), expression->left,
                                                 expression->right, NULL, NULL, "type mismatch in binary expression");
        return;
    }

    TypeInfo *info = NULL;

    // logical ops - bools -> bool
    if (expression->op == TokenType::TOKEN_AND || expression->op == TokenType::TOKEN_OR) {
        if (expression->left->type_info->type != TypeInfoType::BOOLEAN &&
            expression->right->type_info->type != TypeInfoType::BOOLEAN) {
            ErrorReporter::report_type_checker_error(compilation_unit->file_data->absolute_path.string(),
                                                     expression->left, expression->right, NULL, NULL,
                                                     "cannot use logical operators on non bool type");
            return;
        }

        info = this->compilation_unit->global_type_scope["bool"];
    }

    // math ops - numbers -> numbers
    if (expression->op == TokenType::TOKEN_PLUS || expression->op == TokenType::TOKEN_STAR ||
        expression->op == TokenType::TOKEN_SLASH || expression->op == TokenType::TOKEN_MOD ||
        expression->op == TokenType::TOKEN_MINUS) {
        if (expression->left->type_info->type != TypeInfoType::NUMBER) {
            ErrorReporter::report_type_checker_error(compilation_unit->file_data->absolute_path.string(),
                                                     expression->left, expression->right, NULL, NULL,
                                                     "cannot use arithmatic operator on non number");
            return;
        }
        info = expression->left->type_info;
    }

    // math ops - numbers -> bool
    if (expression->op == TokenType::TOKEN_LESS || expression->op == TokenType::TOKEN_GREATER ||
        expression->op == TokenType::TOKEN_GREATER_EQUAL || expression->op == TokenType::TOKEN_LESS_EQUAL) {
        if (expression->left->type_info->type != TypeInfoType::NUMBER) {
            ErrorReporter::report_type_checker_error(compilation_unit->file_data->absolute_path.string(),
                                                     expression->left, expression->right, NULL, NULL,
                                                     "cannot use comparison operator on non number");
            return;
        }
        info = this->compilation_unit->global_type_scope["bool"];
    }

    // compare - any -> bool
    if (expression->op == TokenType::TOKEN_EQUAL || expression->op == TokenType::TOKEN_NOT_EQUAL) {
        info = this->compilation_unit->global_type_scope["bool"];
    }

    assert(info != NULL);

    expression->type_info = info;
}

void TypeChecker::type_check_string_literal_expression(StringLiteralExpression *expression) {
    expression->type_info = new SliceTypeInfo(this->compilation_unit->global_type_scope["u8"]);
    expression->category  = ExpressionCategory::RVALUE;
}

void TypeChecker::type_check_number_literal_expression(NumberLiteralExpression *expression) {
    expression->category      = ExpressionCategory::RVALUE;
    std::string token_string  = this->compilation_unit->get_token_string_from_index(expression->token);

    auto [number, type, size] = extract_number_literal_size(token_string);
    if (size == -1) {
        ErrorReporter::report_type_checker_error(compilation_unit->file_data->absolute_path.string(), expression, NULL,
                                                 NULL, NULL, "problem parsing number literal");
        return;
    }

    // TODO I have no idea what this does
    if (type != NumberType::FLOAT && number != (i64)number) {
        ErrorReporter::report_type_checker_error(compilation_unit->file_data->absolute_path.string(), expression, NULL,
                                                 NULL, NULL, "cannot use decimal point on non float types");
        return;
    }

    expression->number = number;

    switch (type) {
    case NumberType::UNSIGNED: {

        if (number < 0) {
            ErrorReporter::report_type_checker_error(compilation_unit->file_data->absolute_path.string(), expression,
                                                     NULL, NULL, NULL, "unsigned number cannot be negative");
            return;
        }

        if (size == 8) {
            expression->type_info = this->compilation_unit->global_type_scope["u8"];
        } else if (size == 16) {
            expression->type_info = this->compilation_unit->global_type_scope["u16"];
        } else if (size == 32) {
            expression->type_info = this->compilation_unit->global_type_scope["u32"];
        } else if (size == 64) {
            expression->type_info = this->compilation_unit->global_type_scope["u64"];
        }
    } break;
    case NumberType::SIGNED: {

        if (size == 8) {
            expression->type_info = this->compilation_unit->global_type_scope["i8"];
        } else if (size == 16) {
            expression->type_info = this->compilation_unit->global_type_scope["i16"];
        } else if (size == 32) {
            expression->type_info = this->compilation_unit->global_type_scope["i32"];
        } else if (size == 64) {
            expression->type_info = this->compilation_unit->global_type_scope["i64"];
        }
    } break;
    case NumberType::FLOAT: {
        if (size == 8 || size == 16) {
            ErrorReporter::report_type_checker_error(compilation_unit->file_data->absolute_path.string(), expression,
                                                     NULL, NULL, NULL,
                                                     "cannot create float of this size can only use 32 and 64 sizes");
            return;
        } else if (size == 32) {
            expression->type_info = this->compilation_unit->global_type_scope["f32"];
        } else if (size == 64) {
            expression->type_info = this->compilation_unit->global_type_scope["f64"];
        }
    } break;
    }
}

void TypeChecker::type_check_bool_literal_expression(BoolLiteralExpression *expression) {
    expression->type_info = new BoolTypeInfo();
    expression->category  = ExpressionCategory::RVALUE;
}

void TypeChecker::type_check_unary_expression(UnaryExpression *expression) {
    TRY_CALL_VOID(type_check_expression(expression->expression));

    if (expression->op == TokenType::TOKEN_AMPERSAND) {
        expression->type_info = new PointerTypeInfo(expression->expression->type_info);
        expression->category  = ExpressionCategory::RVALUE;
        return;
    }

    if (expression->op == TokenType::TOKEN_STAR) {
        if (expression->expression->type_info->type != TypeInfoType::POINTER) {
            ErrorReporter::report_type_checker_error(compilation_unit->file_data->absolute_path.string(), expression,
                                                     NULL, NULL, NULL, "cannot dereference non-pointer value");
            return;
        }

        expression->type_info = ((PointerTypeInfo *)expression->expression->type_info)->to;
        expression->category  = ExpressionCategory::LVALUE;
        return;
    }

    if (expression->op == TokenType::TOKEN_NOT) {
        if (expression->expression->type_info->type != TypeInfoType::BOOLEAN) {
            ErrorReporter::report_type_checker_error(compilation_unit->file_data->absolute_path.string(), expression,
                                                     NULL, NULL, NULL,
                                                     "cannot use unary operator ! on non-boolean type");
            return;
        }

        expression->type_info = expression->expression->type_info;
        expression->category  = ExpressionCategory::RVALUE;
        return;
    }

    UNREACHABLE();
}

void TypeChecker::type_check_call_expression(CallExpression *expression) {
    expression->category = ExpressionCategory::RVALUE;
    TRY_CALL_VOID(type_check_expression(expression->callee));
    Expression *callee_expression = expression->callee;

    if (expression->callee->type_info->type != TypeInfoType::FN) {
        ErrorReporter::report_type_checker_error(compilation_unit->file_data->absolute_path.string(), callee_expression,
                                                 NULL, NULL, NULL, "can only call functions");
    }

    auto arg_type_infos = std::vector<TypeInfo *>();
    for (auto arg : expression->args) {
        TRY_CALL_VOID(type_check_expression(arg));
        arg_type_infos.push_back(arg->type_info);
    }

    auto fn_type_info = static_cast<FnTypeInfo *>(callee_expression->type_info);
    if (fn_type_info->args.size() != arg_type_infos.size()) {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->absolute_path.string(), callee_expression, NULL, NULL, NULL,
            std::format("incorrect number of arguments in call expression, expected {} got {}",
                        fn_type_info->args.size(), arg_type_infos.size()));
        return;
    }

    for (u64 i = 0; i < fn_type_info->args.size(); i++) {
        if (!type_match(fn_type_info->args.at(i), arg_type_infos.at(i))) {
            ErrorReporter::report_type_checker_error(compilation_unit->file_data->absolute_path.string(),
                                                     callee_expression, expression->args.at(i), NULL, NULL,
                                                     "mismatched types function call");
            return;
        }
    }

    expression->type_info = fn_type_info->return_type;
}

void TypeChecker::type_check_identifier_expression(IdentifierExpression *expression) {
    TypeInfo *type_info = this->get_from_scope(expression->identifier);
    if (type_info == NULL) {
        std::string identifier = this->compilation_unit->get_token_string_from_index(expression->identifier);
        ErrorReporter::report_type_checker_error(compilation_unit->file_data->absolute_path.string(), expression, NULL,
                                                 NULL, NULL, std::format("unrecognized identifier \"{}\"", identifier));
        return;
    }

    expression->type_info = type_info;
    expression->category  = ExpressionCategory::LVALUE;
}

void TypeChecker::type_check_get_expression(GetExpression *expression) {
    expression->category = ExpressionCategory::LVALUE;
    TRY_CALL_VOID(type_check_expression(expression->lhs));

    if (expression->lhs->type_info->type == TypeInfoType::NAMESPACE) {
        NamespaceTypeInfo *namespace_type_info = (NamespaceTypeInfo *)expression->lhs->type_info;
        CompilationUnit   *namespace_compilation_unit =
            this->compilation_bundle->compilation_units[namespace_type_info->compilation_unit_index];

        std::string identifier       = this->compilation_unit->get_token_string_from_index(expression->member);
        TypeInfo   *member_type_info = namespace_compilation_unit->get_fn_from_scope_with_string(identifier);

        if (member_type_info == NULL) {
            ErrorReporter::report_type_checker_error(this->compilation_unit->file_data->absolute_path.string(),
                                                     expression->lhs, NULL, NULL, NULL,
                                                     std::format("no symbol '{}' found in namespace", identifier));
            return;
        }

        expression->type_info = member_type_info;
    }

    TypeInfo *using_type = expression->lhs->type_info;

    // if it is a pointer then we dereference it one layer deep
    // ^T.get() --> T.get()
    // ^^T.get() --> ^T.get() :: won't go all the layers down
    if (expression->lhs->type_info->type == TypeInfoType::POINTER) {
        using_type = ((PointerTypeInfo *)expression->lhs->type_info)->to;
    }

    if (using_type->type == TypeInfoType::STRUCT) {
        StructTypeInfo *struct_type_info = (StructTypeInfo *)using_type;

        std::string member_string        = this->compilation_unit->get_token_string_from_index(expression->member);
        TypeInfo   *member_type_info     = NULL;
        for (auto [identifier, member_type] : struct_type_info->members) {
            if (identifier == member_string) {
                member_type_info = member_type;
                break;
            }
        }

        if (member_type_info == NULL) {
            ErrorReporter::report_type_checker_error(compilation_unit->file_data->absolute_path.string(), expression,
                                                     NULL, NULL, NULL,
                                                     std::format("Cannot find member \"{}\" in struct", member_string));
            return;
        }

        expression->type_info = member_type_info;
        return;
    }

    if (using_type->type == TypeInfoType::STATIC_ARRAY || using_type->type == TypeInfoType::SLICE) {
        std::string member_string = this->compilation_unit->get_token_string_from_index(expression->member);

        if (compare_string(member_string, "size")) {
            expression->type_info = this->compilation_unit->global_type_scope["u64"];
            return;
        }

        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message(std::format(
                "can only use 'size' builtin member for static arrays and slices '{}' does not exist", member_string))
            .set_expr_1(expression)
            .report();

        return;
    }

    ErrorReporter::report_type_checker_error(this->compilation_unit->file_data->absolute_path, expression->lhs, NULL,
                                             NULL, NULL,
                                             "Cannot derive member from non struct/namespace/array/slice type");
    return;
}

void TypeChecker::type_check_group_expression(GroupExpression *expression) {
    TRY_CALL_VOID(type_check_expression(expression->sub_expression));
    expression->type_info = expression->sub_expression->type_info;
    expression->category  = expression->sub_expression->category;
}

void TypeChecker::type_check_null_literal_expression(NullLiteralExpression *expression) {
    expression->type_info = new PointerTypeInfo(new AnyTypeInfo());
    expression->category  = ExpressionCategory::RVALUE;
}

void TypeChecker::type_check_zero_literal_expression(ZeroLiteralExpression *expression) {
    expression->type_info = new AnyTypeInfo{TypeInfoType::ANY};
    expression->category  = ExpressionCategory::RVALUE;
}

void TypeChecker::type_check_struct_instance_expression(StructInstanceExpression *expression) {
    expression->category = ExpressionCategory::RVALUE;
    TRY_CALL_VOID(type_check_type_expression(expression->type_expression));

    TypeInfo *type_info = expression->type_expression->type_info;
    // check it's a struct
    if (type_info->type != TypeInfoType::STRUCT) {
        ErrorReporter::report_type_checker_error(compilation_unit->file_data->absolute_path.string(), expression, NULL,
                                                 NULL, NULL, "Can only use struct types in new expression");
        return;
    }

    auto struct_type_info        = (StructTypeInfo *)type_info;

    // collect members from new constructor
    auto calling_args_type_infos = std::vector<std::tuple<std::string, TypeInfo *>>();
    for (auto [name_token_index, expr] : expression->named_expressions) {
        std::string name = this->compilation_unit->get_token_string_from_index(name_token_index);
        TRY_CALL_VOID(type_check_expression(expr));
        calling_args_type_infos.emplace_back(name, expr->type_info);
    }

    // check counts
    if (struct_type_info->members.size() != calling_args_type_infos.size()) {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->absolute_path.string(), expression, NULL, NULL, NULL,
            std::format("Incorrect number of arguments in new expression, expected {} got {}",
                        struct_type_info->members.size(), calling_args_type_infos.size()));
        return;
    }

    // check types
    for (u64 i = 0; i < calling_args_type_infos.size(); i++) {
        // new expression member and type info
        auto [expression_member, expression_type] = calling_args_type_infos.at(i);

        // struct member and type info
        auto [member, type]                       = struct_type_info->members.at(i);

        if (expression_member != member) {
            auto [name, expr] = expression->named_expressions.at(i);
            ErrorReporter::report_type_checker_error(compilation_unit->file_data->absolute_path.string(), expr, NULL,
                                                     NULL, NULL, "Incorrect name specifier in new expression");
            return;
        }
    }

    expression->type_info = struct_type_info;
}

void TypeChecker::type_check_static_array_literal_expression(StaticArrayExpression *expression) {
    expression->category = ExpressionCategory::RVALUE;
    TRY_CALL_VOID(type_check_expression(expression->number));
    TRY_CALL_VOID(type_check_type_expression(expression->type_expression));

    if (expression->expressions.size() != expression->number->number) {
        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message(std::format("static array literal expects {} values but got {}", expression->number->number,
                                     expression->expressions.size()))
            .set_expr_1(expression->number)
            .set_type_expr_1(expression->type_expression)
            .report();

        return;
    }

    for (Expression *expr : expression->expressions) {
        TRY_CALL_VOID(type_check_expression(expr));
        if (!type_match(expression->type_expression->type_info, expr->type_info)) {
            TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
                .set_message("mistmaatched types in static array literal")
                .set_expr_1(expr)
                .set_type_expr_1(expression->type_expression)
                .report();

            return;
        }
    }

    expression->type_info = new StaticArrayTypeInfo(expression->number->number, expression->type_expression->type_info);
}

void TypeChecker::type_check_subscript_expression(SubscriptExpression *expression) {
    TRY_CALL_VOID(type_check_expression(expression->subscriptee));
    TRY_CALL_VOID(type_check_expression(expression->subscripter));

    if (expression->subscriptee->type_info->type != TypeInfoType::STATIC_ARRAY &&
        expression->subscriptee->type_info->type != TypeInfoType::SLICE) {
        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message("can only subscript array and slice types")
            .set_expr_1(expression->subscriptee)
            .set_expr_2(expression->subscripter)
            .report();

        return;
    }

    if (expression->subscripter->type_info->type != TypeInfoType::NUMBER &&
        expression->subscripter->type_info->type != TypeInfoType::RANGE) {
        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message("can only subscript with number and range expressions")
            .set_expr_2(expression->subscripter)
            .report();

        return;
    }

    if (expression->subscripter->type_info->type == TypeInfoType::NUMBER) {
        // this maybe wrong, what if we are subscripting a r-value??
        // e.g. [3]i64{1, 2, 4}[0] = 3;
        // this shouldn't work right??
        expression->category = ExpressionCategory::LVALUE;
    } else {
        ASSERT(expression->subscripter->type_info->type == TypeInfoType::RANGE);

        // slicing on an array creates a new value so it is a r-value
        expression->category = ExpressionCategory::RVALUE;
    }

    // even though you can slice on an array or a slice we only care about the base
    // type so this code can be generic across both of them. If in the future we
    // have other types that can be sliced but don't fit this pattern we can
    // change it here
    TypeInfo *base_type = NULL;
    if (expression->subscriptee->type_info->type == TypeInfoType::STATIC_ARRAY) {
        StaticArrayTypeInfo *array_type_info = (StaticArrayTypeInfo *)expression->subscriptee->type_info;
        base_type                            = array_type_info->base_type;
    } else if (expression->subscriptee->type_info->type == TypeInfoType::SLICE) {
        SliceTypeInfo *slice_type_info = (SliceTypeInfo *)expression->subscriptee->type_info;
        base_type                      = slice_type_info->base_type;
    }

    { // when the subscripter is a number
        if (expression->subscripter->type_info->type == TypeInfoType::NUMBER) {
            NumberTypeInfo *number_type_info = (NumberTypeInfo *)expression->subscripter->type_info;
            if (number_type_info->number_type == NumberType::FLOAT) {
                TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
                    .set_message("cannot use a float type to subscript an array")
                    .set_expr_2(expression->subscripter)
                    .report();

                return;
            }

            expression->type_info = base_type;
            return;
        }
    }

    { // when the subscripter is a range
        if (expression->subscripter->type_info->type == TypeInfoType::RANGE) {
            expression->type_info = new SliceTypeInfo(base_type);
            return;
        }
    }

    // can only subscript with number types and range types
    UNREACHABLE();
}

void TypeChecker::type_check_range_expression(RangeExpression *expression) {
    expression->category                 = ExpressionCategory::RVALUE;

    // this is kind of a mess but with the lambda you end up repeating so much code that I dont
    // think it is work it and it looks so confusing. If this is slow we can refactor but keeping
    // all of the logic in one place I think is better
    auto expression_can_be_used_in_range = [this](Expression *expr) -> bool {
        if (expr->type_info->type == TypeInfoType::NUMBER &&
            ((NumberTypeInfo *)expr->type_info)->number_type != NumberType::FLOAT) {
            return true;
        }

        TypeCheckerError::make(this->compilation_unit->file_data->absolute_path.string())
            .set_message("range expression can only use non-float number types")
            .set_expr_1(expr)
            .report();
        return false;
    };

    if (expression->start != NULL) {
        TRY_CALL_VOID(type_check_expression(expression->start));
        if (!expression_can_be_used_in_range(expression->start))
            return;
    }

    if (expression->end != NULL) {
        TRY_CALL_VOID(type_check_expression(expression->end));
        if (!expression_can_be_used_in_range(expression->end))
            return;
    }

    expression->type_info = new RangeTypeInfo();
}

void TypeChecker::type_check_type_expression(TypeExpression *type_expression) {
    switch (type_expression->type) {
    case TypeExpressionType::TYPE_IDENTIFIER:
        type_check_identifier_type_expression(static_cast<IdentifierTypeExpression *>(type_expression));
        break;
    case TypeExpressionType::TYPE_UNARY:
        type_check_unary_type_expression(static_cast<UnaryTypeExpression *>(type_expression));
        break;
    case TypeExpressionType::TYPE_GET:
        type_check_get_type_expression(static_cast<GetTypeExpression *>(type_expression));
        break;
    case TypeExpressionType::TYPE_STATIC_ARRAY:
        type_check_static_array_type_expression(static_cast<StaticArrayTypeExpression *>(type_expression));
        break;
    case TypeExpressionType::TYPE_SLICE:
        type_check_slice_type_expression(static_cast<SliceTypeExpression *>(type_expression));
        break;
    default:
        UNREACHABLE();
    }
}

void TypeChecker::type_check_unary_type_expression(UnaryTypeExpression *type_expression) {
    if (type_expression->unary_type == UnaryType::POINTER) {
        TRY_CALL_VOID(type_check_type_expression(type_expression->type_expression));
        type_expression->type_info = new PointerTypeInfo(type_expression->type_expression->type_info);
        return;
    }

    panic("Internal :: Cannot type check this operator yet...");
}

void TypeChecker::type_check_identifier_type_expression(IdentifierTypeExpression *type_expression) {
    // first checking the type table then checking any namespaces
    TypeInfo *type_info = this->compilation_unit->get_type_from_scope(type_expression->identifier);
    if (type_info == NULL) {
        type_info = this->compilation_unit->get_namespace_from_scope(type_expression->identifier);
    }

    if (type_info == NULL) {
        ErrorReporter::report_type_checker_error(
            compilation_unit->file_data->absolute_path.string(), NULL, NULL, type_expression, NULL,
            "unrecognised identifier in type expression, no type or namespace exists with this name");
        return;
    }

    type_expression->type_info = type_info;
}

void TypeChecker::type_check_get_type_expression(GetTypeExpression *type_expression) {
    TRY_CALL_VOID(type_check_type_expression(type_expression->type_expression));
    if (type_expression->type_expression->type_info->type != TypeInfoType::NAMESPACE) {
        ErrorReporter::report_type_checker_error(compilation_unit->file_data->absolute_path.string(), NULL, NULL,
                                                 type_expression->type_expression, NULL,
                                                 "can only use '.' on namespace identifiers in type expressions");
        return;
    }

    NamespaceTypeInfo *namespace_type_info = (NamespaceTypeInfo *)type_expression->type_expression->type_info;
    CompilationUnit   *other_compilation_unit =
        this->compilation_bundle->compilation_units[namespace_type_info->compilation_unit_index];

    // getting the string of the identifier from the current compilation unit
    // but then looking that up in the other compilation unit
    std::string identifier = this->compilation_unit->get_token_string_from_index(type_expression->identifier);
    TypeInfo   *type_info  = other_compilation_unit->get_type_from_scope_with_string(identifier);

    if (type_info == NULL) {
        ErrorReporter::report_type_checker_error(this->compilation_unit->file_data->absolute_path.string(), NULL, NULL,
                                                 type_expression->type_expression, NULL,
                                                 std::format("no symbol '{}' found in namespace", identifier));
        return;
    }

    type_expression->type_info = type_info;
}

void TypeChecker::type_check_static_array_type_expression(StaticArrayTypeExpression *type_expression) {
    TRY_CALL_VOID(type_check_type_expression(type_expression->base_type));
    TRY_CALL_VOID(type_check_number_literal_expression(type_expression->size));

    type_expression->type_info =
        new StaticArrayTypeInfo(type_expression->size->number, type_expression->base_type->type_info);
}

void TypeChecker::type_check_slice_type_expression(SliceTypeExpression *type_expression) {
    TRY_CALL_VOID(type_check_type_expression(type_expression->base_type));

    type_expression->type_info = new SliceTypeInfo(type_expression->base_type->type_info);
}

bool type_match(TypeInfo *a, TypeInfo *b) {

    ASSERT_MSG(!(a->type == TypeInfoType::ANY && b->type == TypeInfoType::ANY), "Cannot compare 2 any types");

    if (a->type != b->type) {
        if (a->type != TypeInfoType::ANY && b->type != TypeInfoType::ANY) {
            return false;
        }
    }

    if (a->type == TypeInfoType::ANY)
        return true;

    if (b->type == TypeInfoType::ANY)
        return true;

    if (a->type == TypeInfoType::VOID || a->type == TypeInfoType::BOOLEAN ||
        a->type == TypeInfoType::STRING) { // values don't matter
        return true;
    } else if (a->type == TypeInfoType::NUMBER) {
        auto int_a = static_cast<NumberTypeInfo *>(a);
        auto int_b = static_cast<NumberTypeInfo *>(b);

        if (int_a->size == int_b->size && int_a->number_type == int_b->number_type)
            return true;

        return false;
    } else if (a->type == TypeInfoType::FN) {
        auto fn_a = static_cast<FnTypeInfo *>(a);
        auto fn_b = static_cast<FnTypeInfo *>(b);

        if (type_match(fn_a->return_type, fn_b->return_type)) {
            for (u64 i = 0; i < fn_a->args.size(); i++) {
                if (!type_match(fn_a->args.at(i), fn_b->args.at(i))) {
                    return false;
                }
            }

            return true;
        }

        return false;
    } else if (a->type == TypeInfoType::STRUCT) {
        auto struct_a = static_cast<StructTypeInfo *>(a);
        auto struct_b = static_cast<StructTypeInfo *>(b);

        // symbol table does not copy struct type infos
        // so pointer to one struct instance of a type
        // is ALWAYS the same as another
        return struct_a == struct_b;
    } else if (a->type == TypeInfoType::POINTER) {
        auto ptr_a = static_cast<PointerTypeInfo *>(a);
        auto ptr_b = static_cast<PointerTypeInfo *>(b);

        return type_match(ptr_a->to, ptr_b->to);
    } else if (a->type == TypeInfoType::STATIC_ARRAY) {
        auto array_a = static_cast<StaticArrayTypeInfo *>(a);
        auto array_b = static_cast<StaticArrayTypeInfo *>(b);

        if (array_a->size != array_b->size) {
            return false;
        }

        return type_match(array_a->base_type, array_b->base_type);
    } else if (a->type == TypeInfoType::SLICE) {
        auto slice_a = static_cast<SliceTypeInfo *>(a);
        auto slice_b = static_cast<SliceTypeInfo *>(b);

        return type_match(slice_a->base_type, slice_b->base_type);
    }

    UNREACHABLE();
    return false;
}

std::tuple<i64, NumberType, i32> extract_number_literal_size(std::string literal) {

#define BAD_PARSE                                                                                                      \
    { 0, NumberType::UNSIGNED, -1 }

    int literal_end = 0;
    while (literal_end < literal.size() &&
           (is_digit(literal.at(literal_end)) || literal.at(literal_end) == '-' || literal.at(literal_end) == '.')) {
        literal_end++;
    }

    // literal == 0..literal_end
    // type == literal_end..literal_end +1
    // postfix == literal_end + 1..string_end

    auto literal_string        = literal.substr(0, literal_end);

    std::string type_string    = "";
    std::string postfix_string = "";

    // if there is a postfix notation
    // else just infer a i64
    if (literal_end != literal.size()) {
        type_string    = literal.substr(literal_end, 1);
        postfix_string = literal.substr(literal_end + 1, literal.size() - literal_end);
    } else {
        auto n = std::stod(literal_string);
        return {n, NumberType::SIGNED, 64};
    }

    int size;

    try {
        size = std::stoi(postfix_string);
    } catch (std::exception &e) {
        return BAD_PARSE;
    }

    NumberType type;

    if (type_string == "u") {
        type = NumberType::UNSIGNED;
    } else if (type_string == "f") {
        type = NumberType::FLOAT;
    } else if (type_string == "i") {
        type = NumberType::SIGNED;
    } else {
        return BAD_PARSE;
    }

    if (size == 8 || size == 16 || size == 32 || size == 64) {
        try {
            auto n = std::stod(literal_string);
            return {n, type, size};
        } catch (std::exception &e) {
        }
    }

    return BAD_PARSE;
}

void add_type_info_to_map(std::vector<SortingNode>                  *nodes,
                          std::unordered_map<StructTypeInfo *, u64> *type_info_to_node_index_map,
                          StructTypeInfo                            *type_info) {
    SortingNode node       = SortingNode(type_info);
    u64         node_index = nodes->size();
    nodes->push_back(node);
    (*type_info_to_node_index_map)[type_info] = node_index;
}

void add_dependent_types_to_list(TypeInfo *type_info, std::vector<StructTypeInfo *> *dependent_types) {
    if (type_info->type == TypeInfoType::STRUCT) {
        dependent_types->push_back((StructTypeInfo *)type_info);
        for (auto [_, child] : ((StructTypeInfo *)type_info)->members) {
            add_dependent_types_to_list(child, dependent_types);
        }
    } else if (type_info->type == TypeInfoType::STATIC_ARRAY) {
        TypeInfo *child_type_info = ((StaticArrayTypeInfo *)type_info)->base_type;
        add_dependent_types_to_list(child_type_info, dependent_types);
    }
}

void resolve_dependencies(std::vector<SortingNode>                  *nodes,
                          std::unordered_map<StructTypeInfo *, u64> *type_info_to_node_index_map,
                          SortingNode                               &current_node) {
    std::vector<StructTypeInfo *> dependent_types;
    for (auto &child : current_node.type_info->members) {
        auto [name, child_type_info] = child;
        add_dependent_types_to_list(child_type_info, &dependent_types);
    }

    for (StructTypeInfo *child : dependent_types) {
        ASSERT(type_info_to_node_index_map->count(child) > 0);

        u64          child_index = (*type_info_to_node_index_map)[child];
        SortingNode *child_node  = &nodes->at(child_index);
        current_node.depends_on.push_back(child_node);
    }
}

void topological_visit(std::vector<SortingNode> *L, SortingNode *node) {
    if (node->permenent_mark)
        return;

    if (node->temperory_mark) {
        // TODO this error is terrible
        CompilationUnit *compilation_unit = node->type_info->defined_location->compilation_unit;
        TypeCheckerError::make(compilation_unit->file_data->absolute_path.string())
            .set_message(std::format("recursive type found in {}", compilation_unit->get_token_string_from_index(
                                                                       node->type_info->defined_location->identifier)))
            .report();

        return;
    }

    node->temperory_mark = true;

    for (SortingNode *child : node->depends_on) {
        topological_visit(L, child);
    }

    node->temperory_mark = false;
    node->permenent_mark = true;
    L->push_back(*node); // TODO maybe we dont need to do a copy here????
}

std::vector<SortingNode> topilogical_sort(std::vector<StructStatement *> statements) {
    std::vector<SortingNode>                  nodes;
    std::unordered_map<StructTypeInfo *, u64> type_info_to_node_index_map;

    // convert all struct statements into the nodes we need for the graph
    for (auto struct_statement : statements) {
        add_type_info_to_map(&nodes, &type_info_to_node_index_map, struct_statement->type_info);
    }

    // fill in the dependency vector for each sorting node
    for (SortingNode &sorting_node : nodes) {
        resolve_dependencies(&nodes, &type_info_to_node_index_map, sorting_node);
    }

    // Depth-first search
    // https://en.wikipedia.org/wiki/Topological_sorting
    std::vector<SortingNode> L;
    for (SortingNode &sorting_node : nodes) {
        topological_visit(&L, &sorting_node);
    }

    return L;
}
