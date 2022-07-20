#include "type_checker.h"

#include <assert.h>

#include "errors.h"
#include "liam.h"
#include "parser.h"

SymbolTable::SymbolTable() {
    this->builtin_type_table = std::map<std::string, TypeInfo *>();
    this->type_table         = std::map<std::string, TypeInfo *>();
    this->identifier_table   = std::map<std::string, TypeInfo *>();

    builtin_type_table["void"] = new VoidTypeInfo{TypeInfoType::VOID};
    builtin_type_table["char"] = new CharTypeInfo{TypeInfoType::CHAR};
    builtin_type_table["u64"]  = new IntTypeInfo{TypeInfoType::INT, false, 64};
    builtin_type_table["bool"] = new BoolTypeInfo{TypeInfoType::BOOLEAN};
    builtin_type_table["str"]  = new StrTypeInfo{TypeInfoType::STRING};
}

void SymbolTable::add_type(Token type, TypeInfo *type_info) {
    if (type_table.contains(type.string))
    {
        panic(
            "Duplcate creation of type: " + type.string + " at (" + std::to_string(type.span.line) + "," +
            std::to_string(type.span.start) + ")"
        );
    }

    type_table[type.string] = type_info;
}

void SymbolTable::add_identifier(Token identifier, TypeInfo *type_info) {
    if (identifier_table.contains(identifier.string))
    {
        panic(
            "Duplcate creation of identifier: " + identifier.string + " at (" + std::to_string(identifier.span.line) + "," +
            std::to_string(identifier.span.start) + ")"
        );
    }

    identifier_table[identifier.string] = type_info;
}

TypeInfo *SymbolTable::get_type(Token *identifier) {
    return get_type(identifier->string);
}

TypeInfo *SymbolTable::get_type(std::string identifier) {
    if (builtin_type_table.contains(identifier))
    { return builtin_type_table[identifier]; }

    if (identifier_table.contains(identifier))
    { return identifier_table[identifier]; }

    if (type_table.contains(identifier))
    { return type_table[identifier]; }

    return nullptr;
}

SymbolTable SymbolTable::copy() {
    SymbolTable st = *this;
    return st;
}

TypeChecker::TypeChecker() {
    symbol_table = SymbolTable();
    current_file = NULL;
}

File TypeChecker::type_check(std::vector<File *> *files) {
    auto typed_file = File("<?>");

    auto structs = std::vector<StructStatement *>();
    auto funcs   = std::vector<FnStatement *>();
    auto others  = std::vector<Statement *>();

    for (auto file : *files)
    {
        for (auto stmt : file->statements)
        {
            if (stmt->statement_type == StatementType::STATEMENT_STRUCT)
            { structs.push_back(dynamic_cast<StructStatement *>(stmt)); }
            else if (stmt->statement_type == StatementType::STATEMENT_FN)
            {
                funcs.push_back(dynamic_cast<FnStatement *>(stmt));
                others.push_back(stmt);
            }
            else
            { others.push_back(stmt); }
        }
    }

    // struct identifier pass
    for (auto stmt : structs)
    { symbol_table.add_type(stmt->identifier, nullptr); }

    // struct type pass
    for (auto stmt : structs)
    {
        current_file = stmt->file;
        type_check_struct_statement(stmt, &symbol_table, true);
        typed_file.statements.push_back(stmt);
    }

    // function decl pass
    for (auto stmt : funcs)
    {
        current_file = stmt->file;
        type_check_fn_decl(stmt, &symbol_table);
    }

    for (auto stmt : others)
    {
        current_file = stmt->file;
        type_check_statement(stmt, &symbol_table);
        typed_file.statements.push_back(stmt);
    }

    return typed_file;
}

void TypeChecker::type_check_fn_decl(FnStatement *statement, SymbolTable *symbol_table) {
    SymbolTable *symbol_table_in_use = symbol_table;
    if (!statement->generics.empty())
    {
        SymbolTable *copy = new SymbolTable();
        *copy             = *symbol_table;
        for (auto &generic : statement->generics)
        {
            copy->add_type(
                generic, new GenericTypeInfo{TypeInfoType::GENERIC, 0}
            ); // TODO: generic ids need to work here
        }

        symbol_table_in_use = copy;
    }

    auto param_type_infos = std::vector<TypeInfo *>();
    for (auto &[identifier, expr] : statement->params)
    {
        TRY_CALL(type_check_type_expression(expr, symbol_table_in_use));
        param_type_infos.push_back(expr->type_info);
    }

    TRY_CALL(type_check_type_expression(statement->return_type, symbol_table_in_use));

    // add this fn decl to parent symbol table
    symbol_table->add_identifier(
        statement->identifier,
        new FnTypeInfo{
            TypeInfoType::FN, statement->return_type->type_info, param_type_infos, statement->generics.size()}
    );
}

void TypeChecker::type_check_statement(Statement *statement, SymbolTable *symbol_table) {
    switch (statement->statement_type)
    {
    case StatementType::STATEMENT_INSERT:
        return type_check_insert_statement(dynamic_cast<InsertStatement *>(statement), symbol_table);
        break;
    case StatementType::STATEMENT_RETURN:
        return type_check_return_statement(dynamic_cast<ReturnStatement *>(statement), symbol_table);
        break;
    case StatementType::STATEMENT_BREAK:
        return type_check_break_statement(dynamic_cast<BreakStatement *>(statement), symbol_table);
        break;
    case StatementType::STATEMENT_FN:
        return type_check_fn_statement(dynamic_cast<FnStatement *>(statement), symbol_table);
        break;
    case StatementType::STATEMENT_LOOP:
        return type_check_loop_statement(dynamic_cast<LoopStatement *>(statement), symbol_table);
        break;
    case StatementType::STATEMENT_STRUCT:
        return type_check_struct_statement(dynamic_cast<StructStatement *>(statement), symbol_table);
        break;
    case StatementType::STATEMENT_ASSIGNMENT:
        return type_check_assigment_statement(dynamic_cast<AssigmentStatement *>(statement), symbol_table);
        break;
    case StatementType::STATEMENT_EXPRESSION:
        return type_check_expression_statement(dynamic_cast<ExpressionStatement *>(statement), symbol_table);
        break;
    case StatementType::STATEMENT_LET:
        return type_check_let_statement(dynamic_cast<LetStatement *>(statement), symbol_table);
        break;
    case StatementType::STATEMENT_FOR:
        return type_check_for_statement(dynamic_cast<ForStatement *>(statement), symbol_table);
        break;
    case StatementType::STATEMENT_IF:
        return type_check_if_statement(dynamic_cast<IfStatement *>(statement), symbol_table);
        break;
    default:
        panic("Statement not implemented in type checker, id -> " + std::to_string((int)statement->statement_type));
    }
}

void TypeChecker::type_check_insert_statement(InsertStatement *statement, SymbolTable *symbol_table) {
    TRY_CALL(type_check_expression(statement->byte_code, symbol_table));
    auto to_compare = PointerTypeInfo{TypeInfoType::POINTER, symbol_table->builtin_type_table["char"]};
    if (!type_match(statement->byte_code->type_info, &to_compare))
    { panic("Insert requires a string"); }
}

void TypeChecker::type_check_return_statement(ReturnStatement *statement, SymbolTable *symbol_table) {
    TRY_CALL(type_check_expression(statement->expression, symbol_table));
}

void TypeChecker::type_check_break_statement(BreakStatement *statement, SymbolTable *symbol_table) {
    panic("Not implemented");
}

void TypeChecker::type_check_let_statement(LetStatement *statement, SymbolTable *symbol_table) {
    TRY_CALL(type_check_expression(statement->rhs, symbol_table));

    // if let type is there type match both and set var type
    // to the let type... else just set it to the rhs
    if (statement->type)
    {
        type_check_type_expression(statement->type, symbol_table);
        if (!type_match(statement->type->type_info, statement->rhs->type_info))
        {
            ErrorReporter::report_type_checker_error(
                current_file->path, statement->rhs, NULL, statement->type, NULL, "Mismatched types in let statement"
            );
            return;
        }

        symbol_table->add_identifier(statement->identifier, statement->type->type_info);
        return;
    }

    symbol_table->add_identifier(statement->identifier, statement->rhs->type_info);
}

void TypeChecker::type_check_scope_statement(
    ScopeStatement *statement, SymbolTable *symbol_table, bool copy_symbol_table
) {
    // this is kind of a mess... oh jeez
    SymbolTable *scopes_symbol_table = symbol_table;
    SymbolTable possible_copy;
    if (copy_symbol_table)
    {
        possible_copy       = *symbol_table;
        scopes_symbol_table = &possible_copy;
    }

    for (auto stmt : statement->statements)
    { TRY_CALL(type_check_statement(stmt, scopes_symbol_table)); }
}
void TypeChecker::type_check_fn_statement(FnStatement *statement, SymbolTable *symbol_table, bool first_pass) {
    if (statement->is_extern)
        return;
    auto existing_type_info = static_cast<FnTypeInfo *>(symbol_table->identifier_table[statement->identifier.string]);

    // params and get return_type expressions
    auto args = std::vector<std::tuple<Token, TypeInfo *>>();
    for (int i = 0; i < existing_type_info->args.size(); i++)
    {
        auto &[identifier, expr] = statement->params.at(i);
        args.push_back({identifier, existing_type_info->args.at(i)});
    }

    // copy table and add params
    SymbolTable copied_symbol_table = *symbol_table;
    for (auto &[identifier, type_info] : args)
    { copied_symbol_table.add_identifier(identifier, type_info); }

    // type statements and check return exists if needed
    TRY_CALL(type_check_scope_statement(statement->body, &copied_symbol_table, false));
    if (existing_type_info->return_type->type == TypeInfoType::VOID)
    {
        for (auto stmt : statement->body->statements)
        {
            if (stmt->statement_type == StatementType::STATEMENT_RETURN)
            { panic("found return statement when return return_type is void"); }
        }
    }
    else
    {
        bool found_return = false;
        for (auto stmt : statement->body->statements)
        {
            if (stmt->statement_type == StatementType::STATEMENT_RETURN)
            {
                found_return          = true;
                auto return_statement = dynamic_cast<ReturnStatement *>(stmt);
                if (!type_match(existing_type_info->return_type, return_statement->expression->type_info))
                {
                    panic("Mismatch types in function, return types do not "
                          "match");
                }
            }
        }

        if (!found_return)
        {
            panic("No return statement in function that has non void return "
                  "return_type");
        }
    }
}

void TypeChecker::type_check_loop_statement(LoopStatement *statement, SymbolTable *symbol_table) {
    TRY_CALL(type_check_scope_statement(statement->body, symbol_table));
}

void TypeChecker::type_check_for_statement(ForStatement *statement, SymbolTable *symbol_table) {
    auto table_copy = *symbol_table;
    TRY_CALL(type_check_let_statement(statement->let_statement, &table_copy));
    TRY_CALL(type_check_expression(statement->condition, &table_copy));
    TRY_CALL(type_check_statement(statement->update, &table_copy));

    if (statement->condition->type_info->type != TypeInfoType::BOOLEAN)
    { panic("Second statement in for loop needs to evaluate to a bool"); }

    TRY_CALL(type_check_scope_statement(statement->body, &table_copy, false));
}

void TypeChecker::type_check_if_statement(IfStatement *statement, SymbolTable *symbol_table) {
    // if may declare new varaibles that we do not want to leak into outher
    // scope like with the is expression
    auto copy = symbol_table->copy();

    TRY_CALL(type_check_expression(statement->expression, &copy));
    if (!type_match(statement->expression->type_info, copy.get_type("bool")))
    { panic("If statement must be passed a bool"); }
    TRY_CALL(type_check_scope_statement(statement->body, &copy));
}

void TypeChecker::type_check_struct_statement(StructStatement *statement, SymbolTable *symbol_table, bool first_pass) {
    SymbolTable sub_symbol_table_copy = symbol_table->copy();

    for (u64 i = 0; i < statement->generics.size(); i++)
    { sub_symbol_table_copy.add_type(statement->generics[i], new GenericTypeInfo{TypeInfoType::GENERIC, i}); }

    auto members_type_info = std::vector<std::tuple<std::string, TypeInfo *>>();
    for (auto &[member, expr] : statement->members)
    {
        TRY_CALL(type_check_type_expression(expr, &sub_symbol_table_copy));
        members_type_info.emplace_back(member.string, expr->type_info);
    }

    if (first_pass)
    {
        symbol_table->type_table[statement->identifier.string] =
            new StructTypeInfo{TypeInfoType::STRUCT, members_type_info, statement->generics.size()};
    }
    else
    {
        symbol_table->add_identifier(
            statement->identifier,
            new StructTypeInfo{TypeInfoType::STRUCT, members_type_info, statement->generics.size()}
        );
    }
}
void TypeChecker::type_check_assigment_statement(AssigmentStatement *statement, SymbolTable *symbol_table) {
    TRY_CALL(type_check_expression(statement->lhs, symbol_table));
    TRY_CALL(type_check_expression(statement->assigned_to->expression, symbol_table));

    if (!type_match(statement->lhs->type_info, statement->assigned_to->expression->type_info))
    {
        panic("Type mismatch, trying to assign a identifier to an expression "
              "of different type");
    }
}

void TypeChecker::type_check_expression_statement(ExpressionStatement *statement, SymbolTable *symbol_table) {
    TRY_CALL(type_check_expression(statement->expression, symbol_table));
}

void TypeChecker::type_check_expression(Expression *expression, SymbolTable *symbol_table) {
    switch (expression->type)
    {
    case ExpressionType::EXPRESSION_STRING_LITERAL:
        return type_check_string_literal_expression(dynamic_cast<StringLiteralExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_INT_LITERAL:
        return type_check_int_literal_expression(dynamic_cast<IntLiteralExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_BOOL_LITERAL:
        return type_check_bool_literal_expression(dynamic_cast<BoolLiteralExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_CALL:
        return type_check_call_expression(dynamic_cast<CallExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_IDENTIFIER:
        return type_check_identifier_expression(dynamic_cast<IdentifierExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_IS:
        return type_check_is_expression(dynamic_cast<IsExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_BINARY:
        return type_check_binary_expression(dynamic_cast<BinaryExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_UNARY:
        return type_check_unary_expression(dynamic_cast<UnaryExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_GET:
        return type_check_get_expression(dynamic_cast<GetExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_NEW:
        return type_check_new_expression(dynamic_cast<NewExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_GROUP:
        return type_check_group_expression(dynamic_cast<GroupExpression *>(expression), symbol_table);
    default:
        panic("Not implemented");
    }
}

void TypeChecker::type_check_is_expression(IsExpression *expression, SymbolTable *symbol_table) {
    TRY_CALL(type_check_expression(expression->expression, symbol_table));
    TRY_CALL(type_check_type_expression(expression->type_expression, symbol_table));

    if (expression->expression->type_info->type != TypeInfoType::UNION)
    { panic("Cannot use is expressio on non union type"); }

    expression->type_info = symbol_table->get_type("bool");
    symbol_table->add_identifier(
        expression->identifier, new PointerTypeInfo{TypeInfoType::POINTER, expression->type_expression->type_info}
    );
}

void TypeChecker::type_check_binary_expression(BinaryExpression *expression, SymbolTable *symbol_table) {
    TRY_CALL(type_check_expression(expression->left, symbol_table));
    TRY_CALL(type_check_expression(expression->right, symbol_table));

    if (!type_match(expression->left->type_info, expression->right->type_info))
    { panic("Type mismatch in binary expression"); }

    TypeInfo *info = nullptr;

    // logical ops - bools -> bool
    if (expression->op.type == TOKEN_AND || expression->op.type == TOKEN_OR)
    {
        if (expression->left->type_info->type != TypeInfoType::BOOLEAN &&
            expression->right->type_info->type != TypeInfoType::BOOLEAN)
        { panic("Cannot use logical operators on non bool type"); }
        info = symbol_table->get_type("bool");
    }

    // math ops - numbers -> numbers
    if (expression->op.type == TOKEN_PLUS || expression->op.type == TOKEN_STAR)
    {
        if (expression->left->type_info->type != TypeInfoType::INT)
        { panic("Cannot use arithmatic operator on non number"); }
        info = expression->left->type_info;
    }

    // math ops - numbers -> bool
    if (expression->op.type == TOKEN_LESS || expression->op.type == TOKEN_GREATER)
    {
        if (expression->left->type_info->type != TypeInfoType::INT)
        { panic("Cannot use arithmatic operator on non number"); }
        info = symbol_table->get_type("bool");
    }

    // compare - any -> bool
    if (expression->op.type == TOKEN_EQUAL || expression->op.type == TOKEN_NOT_EQUAL)
    { info = symbol_table->get_type("bool"); }

    expression->type_info = info;
}
void TypeChecker::type_check_string_literal_expression(StringLiteralExpression *expression, SymbolTable *symbol_table) {
    expression->type_info = symbol_table->builtin_type_table["str"];
}

void TypeChecker::type_check_int_literal_expression(IntLiteralExpression *expression, SymbolTable *symbol_table) {
    expression->type_info = new IntTypeInfo{TypeInfoType::INT, false, 64};
}

void TypeChecker::type_check_bool_literal_expression(BoolLiteralExpression *expression, SymbolTable *symbol_table) {
    expression->type_info = new BoolTypeInfo{TypeInfoType::BOOLEAN};
}

void TypeChecker::type_check_unary_expression(UnaryExpression *expression, SymbolTable *symbol_table) {
    TRY_CALL(type_check_expression(expression->expression, symbol_table));

    TypeInfo *type_info;
    if (expression->op.type == TOKEN_AT)
    { type_info = new PointerTypeInfo{TypeInfoType::POINTER, expression->expression->type_info}; }
    else if (expression->op.type == TOKEN_STAR)
    {
        if (expression->expression->type_info->type != TypeInfoType::POINTER)
        { panic("Cannot dereference non-pointer value"); }

        auto ptr_type = static_cast<PointerTypeInfo *>(expression->expression->type_info);
        type_info     = ptr_type->to;
    }
    else
    { panic("Unrecognised unary operator"); }

    expression->type_info = type_info;
}
void TypeChecker::type_check_call_expression(CallExpression *expression, SymbolTable *symbol_table) {
    TRY_CALL(type_check_expression(expression->identifier, symbol_table));
    auto type_of_callee = dynamic_cast<IdentifierExpression *>(expression->identifier);

    if (!type_of_callee)
    { panic("Can only call on identifier expressions"); }

    if (type_of_callee->type_info->type != TypeInfoType::FN)
    { panic("Can only call a function"); }

    auto arg_type_infos = std::vector<TypeInfo *>();
    for (auto arg : expression->args)
    {
        TRY_CALL(type_check_expression(arg, symbol_table));
        arg_type_infos.push_back(arg->type_info);
    }

    auto fn_type_info = static_cast<FnTypeInfo *>(type_of_callee->type_info);
    if (fn_type_info->args.size() != arg_type_infos.size())
    { panic("Incorrect numbers of args passed to function call"); }

    if (fn_type_info->generic_count != expression->generics.size())
    { panic("Incorrect numbers of type params passed to function call"); }

    for (s32 i = 0; i < fn_type_info->args.size(); i++)
    {
        if (!type_match(fn_type_info->args.at(i), arg_type_infos.at(i)))
        { panic("Type mismatch at function call"); }
    }

    for (u64 i = 0; i < expression->generics.size(); i++)
    { TRY_CALL(type_check_type_expression(expression->generics.at(i), symbol_table)); }

    fn_type_info->return_type = resolve_generics(fn_type_info->return_type, &expression->generics);

    expression->type_info = fn_type_info->return_type;
}
void TypeChecker::type_check_identifier_expression(IdentifierExpression *expression, SymbolTable *symbol_table) {
    TypeInfo *type_info;

    if (symbol_table->builtin_type_table.contains(expression->identifier.string))
    { type_info = symbol_table->builtin_type_table[expression->identifier.string]; }
    else if (symbol_table->type_table.contains(expression->identifier.string))
    { type_info = symbol_table->type_table[expression->identifier.string]; }
    else if (symbol_table->identifier_table.contains(expression->identifier.string))
    { type_info = symbol_table->identifier_table[expression->identifier.string]; }
    else
    { ErrorReporter::report_type_checker_error(current_file->path, expression, NULL, NULL, NULL, "Unrecognized identifier"); }

    expression->type_info = type_info;
}

void TypeChecker::type_check_get_expression(GetExpression *expression, SymbolTable *symbol_table) {
    TRY_CALL(type_check_expression(expression->lhs, symbol_table));

    TypeInfo *member_type_info       = NULL;
    StructTypeInfo *struct_type_info = NULL;

    if (expression->lhs->type_info->type == TypeInfoType::POINTER)
    {
        auto ptr_type_info = static_cast<PointerTypeInfo *>(expression->lhs->type_info);
        if (ptr_type_info->to->type != TypeInfoType::STRUCT_INSTANCE && ptr_type_info->to->type != TypeInfoType::STRUCT)
        { panic("Cannot derive member from non struct type"); }
        else
        { struct_type_info = (StructTypeInfo *)ptr_type_info->to; }
    }
    else if (expression->lhs->type_info->type == TypeInfoType::STRUCT)
    { struct_type_info = static_cast<StructTypeInfo *>(expression->lhs->type_info); }
    else if (expression->lhs->type_info->type == TypeInfoType::STRUCT_INSTANCE)
    {
        auto struct_instance_type_info = static_cast<StructInstanceTypeInfo *>(expression->lhs->type_info);
        assert(struct_instance_type_info->struct_type->type == TypeInfoType::STRUCT);
        struct_type_info = static_cast<StructTypeInfo *>(struct_instance_type_info->struct_type);
    }

    // find the type infp in the struct definition, here it could be a base
    // type or it could be a generic type
    for (auto [identifier, member_type] : struct_type_info->members)
    {
        if (identifier == expression->member.string)
        { member_type_info = member_type; }
    }

    if (member_type_info == NULL)
    { panic("Cannot find member in struct"); }

    // if it is generic then resolve the id of it in the struct instance
    if (member_type_info->type == TypeInfoType::GENERIC)
    {
        if (expression->lhs->type_info->type == TypeInfoType::STRUCT)
        { panic("No generic parameters set for struct type"); }

        auto struct_instance_type_info = static_cast<StructInstanceTypeInfo *>(expression->lhs->type_info);
        auto generic_member_info       = static_cast<GenericTypeInfo *>(member_type_info);
        assert(
            generic_member_info->id >= 0 && generic_member_info->id < struct_instance_type_info->generic_types.size()
        );
        member_type_info = struct_instance_type_info->generic_types[generic_member_info->id];
    }

    expression->type_info = member_type_info;
}

void TypeChecker::type_check_new_expression(NewExpression *expression, SymbolTable *symbol_table) {
    // check its a return_type
    if (!symbol_table->type_table.contains(expression->identifier.string))
    { panic("Unrecognised type in new statement"); }

    // check it's a struct
    if (symbol_table->type_table[expression->identifier.string]->type != TypeInfoType::STRUCT)
    { panic("Cannot use new on non struct identifier"); }

    auto struct_type_info = static_cast<StructTypeInfo *>(symbol_table->type_table[expression->identifier.string]);

    // collect members from new constructor
    auto calling_args_type_infos = std::vector<TypeInfo *>();
    for (auto expr : expression->expressions)
    {
        TRY_CALL(type_check_expression(expr, symbol_table));
        calling_args_type_infos.push_back(expr->type_info);
    }

    // collect generic types from new constructor
    auto generic_type_infos = std::vector<TypeInfo *>();
    for (auto type_expr : expression->generics)
    {
        TRY_CALL(type_check_type_expression(type_expr, symbol_table));
        generic_type_infos.push_back(type_expr->type_info);
    }

    // check counts
    if (struct_type_info->members.size() != calling_args_type_infos.size())
    { panic("Incorrect number of arguments in new constructor"); }

    // check types
    for (s32 i = 0; i < calling_args_type_infos.size(); i++)
    {
        auto [member, type] = struct_type_info->members.at(i);

        auto resolved_member_type = resolve_generics(type, &expression->generics);
        if (!type_match(resolved_member_type, calling_args_type_infos.at(i)))
        { panic("Incorect arguments to new constructor argument " + std::to_string(i)); }

        // if (type->type == TypeInfoType::GENERIC)
        // {
        // auto generic_type = static_cast<GenericTypeInfo *>(type);
        // if (!type_match(members_type_infos.at(i),
        // generic_type_infos[generic_type->id]))
        // {
        // panic("Incorect arguments to new constructor");
        // }
        // }
        // else
        // {
        // if (!type_match(members_type_infos.at(i), type))
        // {
        // panic("Incorect arguments to new constructor");
        // }
        // }
    }

    if (struct_type_info->generic_count > 0)
    {
        expression->type_info =
            new StructInstanceTypeInfo{TypeInfoType::STRUCT_INSTANCE, struct_type_info, generic_type_infos};
    }
    else
    { expression->type_info = struct_type_info; }
}

void TypeChecker::type_check_group_expression(GroupExpression *expression, SymbolTable *symbol_table) {
    TRY_CALL(type_check_expression(expression->expression, symbol_table));
    expression->type_info = expression->expression->type_info;
}

void TypeChecker::type_check_type_expression(TypeExpression *type_expression, SymbolTable *symbol_table) {
    switch (type_expression->type)
    {
    case TypeExpressionType::TYPE_IDENTIFIER:
        type_check_identifier_type_expression(dynamic_cast<IdentifierTypeExpression *>(type_expression), symbol_table);
        break;
    case TypeExpressionType::TYPE_UNION:
        type_check_union_type_expression(dynamic_cast<UnionTypeExpression *>(type_expression), symbol_table);
        break;
    case TypeExpressionType::TYPE_UNARY:
        type_check_unary_type_expression(dynamic_cast<UnaryTypeExpression *>(type_expression), symbol_table);
        break;
    case TypeExpressionType::TYPE_SPECIFIED_GENERICS:
        type_check_specified_generics_type_expression(
            dynamic_cast<SpecifiedGenericsTypeExpression *>(type_expression), symbol_table
        );
        break;
    default:
        panic("Not implemented for type checker");
    }
}

void TypeChecker::type_check_union_type_expression(UnionTypeExpression *type_expression, SymbolTable *symbol_table) {
    auto sub_types = std::vector<TypeInfo *>();
    for (auto sub_type_expression : type_expression->type_expressions)
    {
        TRY_CALL(type_check_type_expression(sub_type_expression, symbol_table));
        sub_types.push_back(sub_type_expression->type_info);
    }

    type_expression->type_info = new UnionTypeInfo{TypeInfoType::UNION, sub_types};
}

void TypeChecker::type_check_unary_type_expression(UnaryTypeExpression *type_expression, SymbolTable *symbol_table) {
    if (type_expression->op.type == TokenType::TOKEN_HAT)
    {
        TRY_CALL(type_check_type_expression(type_expression->type_expression, symbol_table));
        type_expression->type_info =
            new PointerTypeInfo{TypeInfoType::POINTER, type_expression->type_expression->type_info};
        return;
    }

    panic("Cannot type check this operator yet...");
}

void TypeChecker::type_check_specified_generics_type_expression(
    SpecifiedGenericsTypeExpression *type_expression, SymbolTable *symbol_table
) {
    // struct StructInstanceTypeInfo : TypeInfo {
    //      TypeInfo* struct_type;
    //      std::vector<TypeInfo*> generic_types;
    // };

    TRY_CALL(type_check_type_expression(type_expression->struct_type, symbol_table));
    auto generic_types = std::vector<TypeInfo *>();
    for (u64 i = 0; i < type_expression->generics.size(); i++)
    {
        TRY_CALL(type_check_type_expression(type_expression->generics.at(i), symbol_table));
        generic_types.push_back(type_expression->generics.at(i)->type_info);
    }

    type_expression->type_info = new StructInstanceTypeInfo{
        TypeInfoType::STRUCT_INSTANCE, type_expression->struct_type->type_info, generic_types};
}

void TypeChecker::type_check_identifier_type_expression(
    IdentifierTypeExpression *type_expression, SymbolTable *symbol_table
) {
    auto type = symbol_table->get_type(&type_expression->identifier);
    if (type)
    {
        type_expression->type_info = type;
        return;
    }

    panic("Unrecognised type in type expression: " + type_expression->identifier.string);
}

TypeInfo *TypeChecker::resolve_generics(TypeInfo *type_info, std::vector<TypeExpression *> *generic_params) {
    if (type_info->type == TypeInfoType::POINTER)
    {
        auto pointer_type_info = static_cast<PointerTypeInfo *>(type_info);
        pointer_type_info->to  = resolve_generics(pointer_type_info->to, generic_params);
        return pointer_type_info;
    }

    if (type_info->type == TypeInfoType::GENERIC)
    {
        auto generic_type_info = static_cast<GenericTypeInfo *>(type_info);
        return generic_params->at(generic_type_info->id)->type_info;
    }

    if (type_info->type == TypeInfoType::STRUCT_INSTANCE)
    {
        auto instance_type_info = static_cast<StructInstanceTypeInfo *>(type_info);
        for (int i = 0; i < instance_type_info->generic_types.size(); i++)
        {
            instance_type_info->generic_types[i] =
                resolve_generics(instance_type_info->generic_types[i], generic_params);
        }
    }

    return type_info;
}

bool type_match(TypeInfo *a, TypeInfo *b) {
    if (a->type == TypeInfoType::ANY && b->type == TypeInfoType::ANY)
    { panic("cannot match types that are both any, will be fixed xx"); }

    if (a->type == TypeInfoType::GENERIC)
        return true; // TODO: this might be a bug not checking b but not sure
                     // what to do here...

    if (a->type != b->type)
    {
        if (a->type == TypeInfoType::UNION)
        {
            bool contains = false;
            auto union_a  = static_cast<UnionTypeInfo *>(a);

            for (auto type_info : union_a->types)
            {
                if (type_match(type_info, b))
                {
                    contains = true;
                    break;
                }
            }

            return contains;
        }

        if (a->type != TypeInfoType::ANY && b->type != TypeInfoType::ANY)
        { return false; }
    }

    if (a->type == TypeInfoType::ANY)
        return true;
    if (b->type == TypeInfoType::ANY)
        return true;

    if (a->type == TypeInfoType::VOID || a->type == TypeInfoType::BOOLEAN || a->type == TypeInfoType::CHAR ||
        a->type == TypeInfoType::STRING)
    { // values don't matter
        return true;
    }
    else if (a->type == TypeInfoType::INT)
    {
        auto int_a = static_cast<IntTypeInfo *>(a);
        auto int_b = static_cast<IntTypeInfo *>(b);

        if (int_a->is_signed == int_b->is_signed && int_a->size == int_b->size)
            return true;

        return false;
    }
    else if (a->type == TypeInfoType::FN)
    {
        auto fn_a = static_cast<FnTypeInfo *>(a);
        auto fn_b = static_cast<FnTypeInfo *>(b);

        if (type_match(fn_a->return_type, fn_b->return_type))
        {
            for (s32 i = 0; i < fn_a->args.size(); i++)
            {
                if (type_match(fn_a->args.at(i), fn_b->args.at(i)))
                {}
                else
                { return false; }
            }

            return true;
        }

        return false;
    }
    else if (a->type == TypeInfoType::STRUCT)
    {
        auto struct_a = static_cast<StructTypeInfo *>(a);
        auto struct_b = static_cast<StructTypeInfo *>(b);

        for (s32 i = 0; i < struct_a->members.size(); i++)
        {
            auto [a_member, a_type] = struct_a->members.at(i);
            auto [b_member, b_type] = struct_b->members.at(i);
            if (!type_match(a_type, b_type))
            { return false; }
        }

        return true;
    }
    else if (a->type == TypeInfoType::POINTER)
    {
        auto ptr_a = static_cast<PointerTypeInfo *>(a);
        auto ptr_b = static_cast<PointerTypeInfo *>(b);

        if (ptr_a->to->type == TypeInfoType::VOID)
            return true; // void^ can be equal to T^, not other way around

        return type_match(ptr_a->to, ptr_b->to);
    }
    else if (a->type == TypeInfoType::STRUCT_INSTANCE)
    {
        auto ptr_a = static_cast<StructInstanceTypeInfo *>(a);
        auto ptr_b = static_cast<StructInstanceTypeInfo *>(b);
        if (!type_match(ptr_a->struct_type, ptr_b->struct_type))
        { return false; }

        for (u64 i = 0; i < ptr_a->generic_types.size(); i++)
        {
            if (!type_match(ptr_a->generic_types.at(i), ptr_b->generic_types.at(i)))
            { return false; }
        }

        return true;
    }
    else if (a->type == TypeInfoType::UNION)
    {
        auto union_a = static_cast<UnionTypeInfo *>(a);
        auto union_b = static_cast<UnionTypeInfo *>(b);

        if (union_a->types.size() != union_b->types.size())
            return false;

        for (int i = 0; i < union_a->types.size(); i++)
        {
            if (!type_match(union_a->types.at(i), union_b->types.at(i)))
                return false;
        }

        return true;
    }

    panic("Cannot type check this type info");
    return false;
}