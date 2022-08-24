#include "type_checker.h"

#include <assert.h>

#include "args.h"
#include "errors.h"
#include "fmt/core.h"
#include "liam.h"
#include "parser.h"
#include "utils.h"

SymbolTable::SymbolTable() {
    this->builtin_type_table = std::map<std::string, TypeInfo *>();
    this->type_table         = std::map<std::string, TypeInfo *>();
    this->identifier_table   = std::map<std::string, TypeInfo *>();

    builtin_type_table["void"] = new VoidTypeInfo{TypeInfoType::VOID};
    builtin_type_table["bool"] = new BoolTypeInfo{TypeInfoType::BOOLEAN};
    builtin_type_table["str"]  = new StrTypeInfo{TypeInfoType::STRING};

    builtin_type_table["u8"] = new NumberTypeInfo{TypeInfoType::NUMBER, 8, UNSIGNED};
    builtin_type_table["s8"] = new NumberTypeInfo{TypeInfoType::NUMBER, 8, SIGNED};

    builtin_type_table["u16"] = new NumberTypeInfo{TypeInfoType::NUMBER, 16, UNSIGNED};
    builtin_type_table["s16"] = new NumberTypeInfo{TypeInfoType::NUMBER, 16, SIGNED};

    builtin_type_table["u32"] = new NumberTypeInfo{TypeInfoType::NUMBER, 32, UNSIGNED};
    builtin_type_table["s32"] = new NumberTypeInfo{TypeInfoType::NUMBER, 32, SIGNED};
    builtin_type_table["f32"] = new NumberTypeInfo{TypeInfoType::NUMBER, 32, FLOAT};

    builtin_type_table["u64"] = new NumberTypeInfo{TypeInfoType::NUMBER, 64, UNSIGNED};
    builtin_type_table["s64"] = new NumberTypeInfo{TypeInfoType::NUMBER, 64, SIGNED};
    builtin_type_table["f64"] = new NumberTypeInfo{TypeInfoType::NUMBER, 64, FLOAT};
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
            "Duplcate creation of identifier: " + identifier.string + " at (" + std::to_string(identifier.span.line) +
            "," + std::to_string(identifier.span.start) + ")"
        );
    }

    identifier_table[identifier.string] = type_info;
}

std::tuple<TypeInfo *, bool> SymbolTable::get_type(Token *identifier) {
    return get_type(identifier->string);
}

std::tuple<TypeInfo *, bool> SymbolTable::get_type(std::string identifier) {
    if (builtin_type_table.contains(identifier))
    { return {builtin_type_table[identifier], false}; }

    if (identifier_table.contains(identifier))
    { return {identifier_table[identifier], false}; }

    if (type_table.contains(identifier))
    { return {type_table[identifier], false}; }

    return {nullptr, true};
}

SymbolTable SymbolTable::copy() {
    SymbolTable st = *this;
    return st;
}

TypeChecker::TypeChecker() {
    top_level_symbol_table = SymbolTable();
    current_file           = NULL;
}

File TypeChecker::type_check(std::vector<File *> *files) {
    auto typed_file = File("<?>");

    auto structs = std::vector<StructStatement *>();
    auto aliases = std::vector<AliasStatement *>();
    auto funcs   = std::vector<FnStatement *>();
    auto enums   = std::vector<EnumStatement *>();
    auto others  = std::vector<Statement *>();

    for (auto file : *files)
    {
        for (auto stmt : file->statements)
        {
            if (stmt->statement_type == StatementType::STATEMENT_STRUCT)
            { structs.push_back(dynamic_cast<StructStatement *>(stmt)); }
            else if (stmt->statement_type == StatementType::STATEMENT_ALIAS)
            {
                // adds the type to the symbol table but keeps
                // it around to code gen it
                aliases.push_back(dynamic_cast<AliasStatement *>(stmt));
            }
            else if (stmt->statement_type == StatementType::STATEMENT_ENUM)
            { enums.push_back(dynamic_cast<EnumStatement *>(stmt)); }
            else if (stmt->statement_type == StatementType::STATEMENT_FN)
            {
                funcs.push_back(dynamic_cast<FnStatement *>(stmt));
                others.push_back(stmt);
            }
            else
            { others.push_back(stmt); }
        }
    }

    // enum type pass
    for (auto stmt : enums)
    {
        current_file = stmt->file;
        type_check_enum_statement(stmt, &top_level_symbol_table);
        typed_file.statements.push_back(stmt);
    }

    // struct identifier pass
    for (auto stmt : structs)
    { type_check_struct_decl(stmt, &top_level_symbol_table); }

    // typedefs
    for (auto stmt : aliases)
    {
        type_check_alias_statement(stmt, &top_level_symbol_table);
        typed_file.statements.push_back(stmt);
    }

    // struct type pass
    for (auto stmt : structs)
    {
        current_file = stmt->file;
        type_check_struct_statement(stmt, &top_level_symbol_table, true);
        typed_file.statements.push_back(stmt);
    }

    // function decl pass
    for (auto stmt : funcs)
    {
        current_file = stmt->file;
        type_check_fn_decl(stmt, &top_level_symbol_table);
    }

    for (auto stmt : others)
    {
        current_file = stmt->file;
        type_check_statement(stmt, &top_level_symbol_table, true);
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
        TRY_CALL(type_check_type_expression(expr, symbol_table_in_use))
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

void TypeChecker::type_check_struct_decl(StructStatement *statement, SymbolTable *symbol_table) {
    symbol_table->add_type(statement->identifier, new UndefinedTypeInfo{TypeInfoType::UNDEFINED});
}

void TypeChecker::type_check_statement(Statement *statement, SymbolTable *symbol_table, bool top_level) {
    switch (statement->statement_type)
    {
    case StatementType::STATEMENT_INSERT:
        return type_check_insert_statement(dynamic_cast<InsertStatement *>(statement), symbol_table);
    case StatementType::STATEMENT_RETURN:
        return type_check_return_statement(dynamic_cast<ReturnStatement *>(statement), symbol_table);
    case StatementType::STATEMENT_BREAK:
        return type_check_break_statement(dynamic_cast<BreakStatement *>(statement), symbol_table);
    case StatementType::STATEMENT_FN:
        return type_check_fn_statement(dynamic_cast<FnStatement *>(statement), symbol_table, top_level);
    case StatementType::STATEMENT_STRUCT:
        // if it is a top level struct it means it is already defined in the symbol table
        // by the first and second pass, this stage is the thrid pass which means we would
        // be re-declaring the struct
        if (!top_level)
            return type_check_struct_statement(dynamic_cast<StructStatement *>(statement), symbol_table);
        break;
    case StatementType::STATEMENT_ASSIGNMENT:
        return type_check_assigment_statement(dynamic_cast<AssigmentStatement *>(statement), symbol_table);
    case StatementType::STATEMENT_EXPRESSION:
        return type_check_expression_statement(dynamic_cast<ExpressionStatement *>(statement), symbol_table);
    case StatementType::STATEMENT_LET:
        return type_check_let_statement(dynamic_cast<LetStatement *>(statement), symbol_table);
    case StatementType::STATEMENT_FOR:
        return type_check_for_statement(dynamic_cast<ForStatement *>(statement), symbol_table);
    case StatementType::STATEMENT_IF:
        return type_check_if_statement(dynamic_cast<IfStatement *>(statement), symbol_table);
    case StatementType::STATEMENT_CONTINUE:
        break;
    case StatementType::STATEMENT_ALIAS:
        return type_check_alias_statement(dynamic_cast<AliasStatement *>(statement), symbol_table);
    case StatementType::STATEMENT_TEST:
        return type_check_test_statement(dynamic_cast<TestStatement *>(statement), symbol_table);
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
    if (statement->expression)
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
        TRY_CALL(type_check_type_expression(statement->type, symbol_table));
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
void TypeChecker::type_check_fn_statement(FnStatement *statement, SymbolTable *symbol_table, bool top_level) {
    if (statement->is_extern)
        return;

    // fn decl is called at all top level function statements, this means if
    // this statement is a top level statement there is no need to repeat
    // but if it is not then do the function decl
    if (!top_level)
    { type_check_fn_decl(statement, symbol_table); }

    auto fn_type_info = static_cast<FnTypeInfo *>(symbol_table->identifier_table[statement->identifier.string]);

    // params and get type expressions
    auto args = std::vector<std::tuple<Token, TypeInfo *>>();
    for (int i = 0; i < fn_type_info->args.size(); i++)
    {
        auto &[identifier, expr] = statement->params.at(i);
        args.push_back({identifier, fn_type_info->args.at(i)});
    }

    // copy table and add params
    SymbolTable copied_symbol_table = *symbol_table;
    for (auto &[identifier, type_info] : args)
    { copied_symbol_table.add_identifier(identifier, type_info); }

    if (statement->identifier.string == "eval_is_expression")
    {
        if (true)
        {}
    }

    // type statements and check return exists if needed
    TRY_CALL(type_check_scope_statement(statement->body, &copied_symbol_table, false));
    for (auto stmt : statement->body->statements)
    {
        if (stmt->statement_type == StatementType::STATEMENT_RETURN)
        {
            auto rt = static_cast<ReturnStatement *>(stmt);

            if (fn_type_info->return_type->type == TypeInfoType::VOID)
            {
                if (rt->expression != NULL)
                {
                    ErrorReporter::report_type_checker_error(
                        current_file->path, rt->expression, NULL, NULL, NULL,
                        "found expression in return when return type is void"
                    );
                    return;
                }
            }
            else
            {
                if (!type_match(fn_type_info->return_type, rt->expression->type_info))
                {
                    ErrorReporter::report_type_checker_error(
                        current_file->path, rt->expression, NULL, statement->return_type, NULL,
                        "Mismatch types in function, return types do not match"
                    );
                    return;
                }
            }
        }
    }
}

void TypeChecker::type_check_for_statement(ForStatement *statement, SymbolTable *symbol_table) {
    auto table_copy = *symbol_table;
    TRY_CALL(type_check_statement(statement->assign, &table_copy));
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

    if (!type_match(statement->expression->type_info, symbol_table->builtin_type_table["bool"]))
    { panic("If statement must be passed a bool"); }

    TRY_CALL(type_check_scope_statement(statement->body, &copy));

    if (statement->else_statement)
    {
        // might not have else statement
        // not using symbol table copy either as symbols
        // should not leak to sub statements
        TRY_CALL(type_check_else_statement(statement->else_statement, symbol_table));
    }
}

void TypeChecker::type_check_else_statement(ElseStatement *statement, SymbolTable *symbol_table) {
    if (statement->if_statement)
    { TRY_CALL(type_check_if_statement(statement->if_statement, symbol_table)); }

    if (statement->body)
    { TRY_CALL(type_check_scope_statement(statement->body, symbol_table)); }
}

void TypeChecker::type_check_struct_statement(StructStatement *statement, SymbolTable *symbol_table, bool top_level) {

    SymbolTable sub_symbol_table_copy = symbol_table->copy();

    for (u64 i = 0; i < statement->generics.size(); i++)
    { sub_symbol_table_copy.add_type(statement->generics[i], new GenericTypeInfo{TypeInfoType::GENERIC, i}); }

    auto members_type_info = std::vector<std::tuple<std::string, TypeInfo *>>();
    for (auto &[member, expr] : statement->members)
    {
        TRY_CALL(type_check_type_expression(expr, &sub_symbol_table_copy));
        members_type_info.emplace_back(member.string, expr->type_info);
    }

    // if this is a top level struct it means its identifier is already in the symbol table from the first pass
    // and we are currently in the second pass, if so then just added the type info to the structs already
    // existing identifier in the table instead of added a new one
    if (top_level)
    {
        // struct type in undefined need to cast it to a struct into * and copy data in [ :
        StructTypeInfo *sti = (StructTypeInfo *)symbol_table->type_table[statement->identifier.string];
        *sti                = StructTypeInfo{TypeInfoType::STRUCT, members_type_info, statement->generics.size()};
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
        ErrorReporter::report_type_checker_error(
            current_file->path, statement->lhs, statement->assigned_to->expression, NULL, NULL,
            "Type mismatch, trying to assign a identifier to an expression of different type"
        );
        return;
    }
}

void TypeChecker::type_check_expression_statement(ExpressionStatement *statement, SymbolTable *symbol_table) {
    TRY_CALL(type_check_expression(statement->expression, symbol_table));
}

void TypeChecker::type_check_enum_statement(EnumStatement *statement, SymbolTable *symbol_table) {
    auto instances = std::vector<std::string>(statement->instances.size());

    for (auto &t : statement->instances)
    { instances.push_back(t.string); }

    symbol_table->add_type(
        statement->identifier, new EnumTypeInfo{TypeInfoType::ENUM, statement->identifier.string, instances}
    );
}

void TypeChecker::type_check_alias_statement(AliasStatement *statement, SymbolTable *symbol_table) {
    TRY_CALL(type_check_type_expression(statement->type_expression, symbol_table));
    symbol_table->add_type(statement->identifier, statement->type_expression->type_info);
}

void TypeChecker::type_check_test_statement(TestStatement *statement, SymbolTable *symbol_table) {

    if (!args->test)
        return; // not a test build no need to type check

    TRY_CALL(type_check_scope_statement(statement->tests, symbol_table));

    for (auto sub_stmt : statement->tests->statements)
    {
        if (sub_stmt->statement_type != StatementType::STATEMENT_FN)
        { panic("Can only declare functions within test statement bodies"); }

        auto fn = (FnStatement *)sub_stmt;
        if (!fn->params.empty())
        { panic("Functions cannot have parameters in test bodies"); }
    }
}

void TypeChecker::type_check_expression(Expression *expression, SymbolTable *symbol_table) {
    switch (expression->type)
    {
    case ExpressionType::EXPRESSION_STRING_LITERAL:
        return type_check_string_literal_expression(dynamic_cast<StringLiteralExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_NUMBER_LITERAL:
        return type_check_number_literal_expression(dynamic_cast<NumberLiteralExpression *>(expression), symbol_table);
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
        break;
    case ExpressionType::EXPRESSION_NULL_LITERAL:
        return type_check_null_literal_expression(dynamic_cast<NullLiteralExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_PROPAGATE:
        return type_check_propagation_expression(dynamic_cast<PropagateExpression *>(expression), symbol_table);
        break;
    case ExpressionType::EXPRESSION_ZERO_LITERAL:
        return type_check_zero_literal_expression(dynamic_cast<ZeroLiteralExpression *>(expression), symbol_table);
        break;
    default:
        panic("Expression not implemented in type checker");
    }
}

void TypeChecker::type_check_is_expression(IsExpression *expression, SymbolTable *symbol_table) {
    TRY_CALL(type_check_expression(expression->expression, symbol_table));
    TRY_CALL(type_check_type_expression(expression->type_expression, symbol_table));

    if (expression->expression->type_info->type != TypeInfoType::UNION)
    {
        ErrorReporter::report_type_checker_error(
            current_file->path, expression->expression, NULL, NULL, NULL,
            "Cannot use non-type-union type in is expression"
        );
        return;
    }

    // check if the type expression given is valid for this type union
    auto union_type_info = (UnionTypeInfo *)expression->expression->type_info;
    bool match           = false;
    for (auto t : union_type_info->types)
    {
        if (type_match(t, expression->type_expression->type_info))
        {
            match = true;
            break;
        }
    }

    if (!match)
    {
        ErrorReporter::report_type_checker_error(
            current_file->path, expression->expression, NULL, expression->type_expression, NULL,
            "Type must be a valid sub type of the type union"
        );
        return;
    }

    expression->type_info = symbol_table->builtin_type_table["bool"];
    symbol_table->add_identifier(
        expression->identifier, new PointerTypeInfo{TypeInfoType::POINTER, expression->type_expression->type_info}
    );
}

void TypeChecker::type_check_binary_expression(BinaryExpression *expression, SymbolTable *symbol_table) {
    TRY_CALL(type_check_expression(expression->left, symbol_table));
    TRY_CALL(type_check_expression(expression->right, symbol_table));

    if (!type_match(expression->left->type_info, expression->right->type_info))
    {
        ErrorReporter::report_type_checker_error(
            current_file->path, expression->left, expression->right, NULL, NULL, "Type mismatch in binary expression"
        );
        return;
    }

    TypeInfo *info = NULL;

    // logical ops - bools -> bool
    if (expression->op.type == TOKEN_AND || expression->op.type == TOKEN_OR)
    {
        if (expression->left->type_info->type != TypeInfoType::BOOLEAN &&
            expression->right->type_info->type != TypeInfoType::BOOLEAN)
        {
            ErrorReporter::report_type_checker_error(
                current_file->path, expression->left, expression->right, NULL, NULL,
                "Cannot use logical operators on non bool type"
            );
            return;
        }

        info = symbol_table->builtin_type_table["bool"];
    }

    // math ops - numbers -> numbers
    if (expression->op.type == TOKEN_PLUS || expression->op.type == TOKEN_STAR || expression->op.type == TOKEN_SLASH ||
        expression->op.type == TOKEN_MOD || expression->op.type == TOKEN_MINUS)
    {
        if (expression->left->type_info->type != TypeInfoType::NUMBER)
        {
            ErrorReporter::report_type_checker_error(
                current_file->path, expression->left, expression->right, NULL, NULL,
                "Cannot use arithmatic operator on non number"
            );
            return;
        }
        info = expression->left->type_info;
    }

    // math ops - numbers -> bool
    if (expression->op.type == TOKEN_LESS || expression->op.type == TOKEN_GREATER ||
        expression->op.type == TOKEN_GREATER_EQUAL || expression->op.type == TOKEN_LESS_EQUAL)
    {
        if (expression->left->type_info->type != TypeInfoType::NUMBER)
        {
            ErrorReporter::report_type_checker_error(
                current_file->path, expression->left, expression->right, NULL, NULL,
                "Cannot use comparison operator on non number"
            );
            return;
        }
        info = symbol_table->builtin_type_table["bool"];
    }

    // compare - any -> bool
    if (expression->op.type == TOKEN_EQUAL || expression->op.type == TOKEN_NOT_EQUAL)
    { info = symbol_table->builtin_type_table["bool"]; }

    assert(info != NULL);

    expression->type_info = info;
}
void TypeChecker::type_check_string_literal_expression(StringLiteralExpression *expression, SymbolTable *symbol_table) {
    expression->type_info = symbol_table->builtin_type_table["str"];
}

void TypeChecker::type_check_number_literal_expression(NumberLiteralExpression *expression, SymbolTable *symbol_table) {

    auto [number, type, size] = extract_number_literal_size(expression->token.string);

    if (size == -1)
    {
        ErrorReporter::report_type_checker_error(
            current_file->path, expression, NULL, NULL, NULL, "Problem parsing number literal"
        );
        return;
    }

    if (type != FLOAT && number != (s64)number)
    {
        ErrorReporter::report_type_checker_error(
            current_file->path, expression, NULL, NULL, NULL, "Cannot use decimal point on non float types"
        );
        return;
    }

    expression->number = number;

    switch (type)
    {
    case UNSIGNED: {

        if (number < 0)
        {
            ErrorReporter::report_type_checker_error(
                current_file->path, expression, NULL, NULL, NULL, "Unsigned number cannot be negative"
            );
            return;
        }

        if (size == 8)
        { expression->type_info = symbol_table->builtin_type_table["u8"]; }
        else if (size == 16)
        { expression->type_info = symbol_table->builtin_type_table["u16"]; }
        else if (size == 32)
        { expression->type_info = symbol_table->builtin_type_table["u32"]; }
        else if (size == 64)
        { expression->type_info = symbol_table->builtin_type_table["u64"]; }
    }
    break;
    case SIGNED: {

        if (size == 8)
        { expression->type_info = symbol_table->builtin_type_table["s8"]; }
        else if (size == 16)
        { expression->type_info = symbol_table->builtin_type_table["s16"]; }
        else if (size == 32)
        { expression->type_info = symbol_table->builtin_type_table["s32"]; }
        else if (size == 64)
        { expression->type_info = symbol_table->builtin_type_table["s64"]; }
    }
    break;
    case FLOAT: {
        if (size == 8 || size == 16)
        {
            ErrorReporter::report_type_checker_error(
                current_file->path, expression, NULL, NULL, NULL,
                "Cannot create float of this size can only use 32 and 64 sizes"
            );
            return;
        }
        else if (size == 32)
        { expression->type_info = symbol_table->builtin_type_table["f32"]; }
        else if (size == 64)
        { expression->type_info = symbol_table->builtin_type_table["f64"]; }
    }
    break;
    }
}

void TypeChecker::type_check_bool_literal_expression(BoolLiteralExpression *expression, SymbolTable *symbol_table) {
    expression->type_info = new BoolTypeInfo{TypeInfoType::BOOLEAN};
}

void TypeChecker::type_check_unary_expression(UnaryExpression *expression, SymbolTable *symbol_table) {
    TRY_CALL(type_check_expression(expression->expression, symbol_table));

    if (expression->op.type == TOKEN_AT)
    {
        expression->type_info = new PointerTypeInfo{TypeInfoType::POINTER, expression->expression->type_info};
        return;
    }

    if (expression->op.type == TOKEN_STAR)
    {
        if (expression->expression->type_info->type != TypeInfoType::POINTER)
        {
            ErrorReporter::report_type_checker_error(
                current_file->path, expression, NULL, NULL, NULL, "Cannot dereference non-pointer value"
            );
            return;
        }

        expression->type_info = static_cast<PointerTypeInfo *>(expression->expression->type_info)->to;
        return;
    }

    if (expression->op.type == TOKEN_NOT)
    {
        if (expression->expression->type_info->type != TypeInfoType::BOOLEAN)
        {
            ErrorReporter::report_type_checker_error(
                current_file->path, expression, NULL, NULL, NULL, "Cannot use unary operator ! on non-boolean type"
            );
            return;
        }

        expression->type_info = expression->expression->type_info;
        return;
    }

    panic("Internal :: Unrecognised unary operator");
}
void TypeChecker::type_check_call_expression(CallExpression *expression, SymbolTable *symbol_table) {
    TRY_CALL(type_check_expression(expression->identifier, symbol_table));
    auto type_of_callee = dynamic_cast<IdentifierExpression *>(expression->identifier);

    if (!type_of_callee)
    {
        ErrorReporter::report_type_checker_error(
            current_file->path, expression->identifier, NULL, NULL, NULL, "Can only call identifier expressions"
        );
        return;
    }

    if (type_of_callee->type_info->type != TypeInfoType::FN)
    {
        ErrorReporter::report_type_checker_error(
            current_file->path, type_of_callee, NULL, NULL, NULL, "Can only call function types"
        );
        return;
    }

    auto arg_type_infos = std::vector<TypeInfo *>();
    for (auto arg : expression->args)
    {
        TRY_CALL(type_check_expression(arg, symbol_table));
        arg_type_infos.push_back(arg->type_info);
    }

    auto fn_type_info = static_cast<FnTypeInfo *>(type_of_callee->type_info);
    if (fn_type_info->args.size() != arg_type_infos.size())
    {
        ErrorReporter::report_type_checker_error(
            current_file->path, type_of_callee, NULL, NULL, NULL,
            fmt::format(
                "Incorrect number of arguments in call expression, expected {} got {}", fn_type_info->args.size(),
                arg_type_infos.size()
            )
        );
        return;
    }

    if (fn_type_info->generic_count != expression->generics.size())
    {
        ErrorReporter::report_type_checker_error(
            current_file->path, type_of_callee, NULL, NULL, NULL,
            fmt::format(
                "Incorrect number of generic arguments in call expression, expected {} got {}",
                fn_type_info->generic_count, expression->generics.size()
            )
        );
        return;
    }

    for (s32 i = 0; i < fn_type_info->args.size(); i++)
    {
        if (!type_match(fn_type_info->args.at(i), arg_type_infos.at(i)))
        {
            ErrorReporter::report_type_checker_error(
                current_file->path, type_of_callee, expression->args.at(i), NULL, NULL, "Mismatched types function call"
            );
            return;
        }
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
    {
        ErrorReporter::report_type_checker_error(
            current_file->path, expression, NULL, NULL, NULL,
            fmt::format("Unrecognized identifier \"{}\"", expression->identifier.string)
        );
    }

    expression->type_info = type_info;
}

void TypeChecker::type_check_get_expression(GetExpression *expression, SymbolTable *symbol_table) {
    TRY_CALL(type_check_expression(expression->lhs, symbol_table));

    if (expression->lhs->type_info->type == TypeInfoType::ENUM)
    {
        expression->type_info = expression->lhs->type_info;
        return;
    }

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
        {
            member_type_info = member_type;
            break;
        }
    }

    if (member_type_info == NULL)
    {
        ErrorReporter::report_type_checker_error(
            current_file->path, expression, NULL, NULL, NULL,
            fmt::format("Cannot find member \"{}\" in struct", expression->member.string)
        );

        return;
    }

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
    // check its type
    if (!symbol_table->type_table.contains(expression->identifier.string))
    {
        ErrorReporter::report_type_checker_error(
            current_file->path, expression, NULL, NULL, NULL, "Unrecognised type in new expression"
        );
        return;
    }

    // check it's a struct
    if (symbol_table->type_table[expression->identifier.string]->type != TypeInfoType::STRUCT)
    {
        ErrorReporter::report_type_checker_error(
            current_file->path, expression, NULL, NULL, NULL, "Can only use struct types in new expression"
        );
        return;
    }

    auto struct_type_info = static_cast<StructTypeInfo *>(symbol_table->type_table[expression->identifier.string]);

    // collect members from new constructor
    auto calling_args_type_infos = std::vector<std::tuple<std::string, TypeInfo *>>();
    for (auto [name, expr] : expression->named_expressions)
    {
        TRY_CALL(type_check_expression(expr, symbol_table));
        calling_args_type_infos.emplace_back(name.string, expr->type_info);
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
    {
        ErrorReporter::report_type_checker_error(
            current_file->path, expression, NULL, NULL, NULL,
            fmt::format(
                "Incorrect number of arguments in new expression, expected {} got {}", struct_type_info->members.size(),
                calling_args_type_infos.size()
            )
        );
        return;
    }

    // check types
    for (s32 i = 0; i < calling_args_type_infos.size(); i++)
    {
        // new expression member and type info
        auto [expression_member, expression_type] = calling_args_type_infos.at(i);

        // struct member and type info
        auto [member, type] = struct_type_info->members.at(i);

        auto resolved_member_type = resolve_generics(type, &expression->generics);
        if (expression_member != member)
        {
            auto [name, expr] = expression->named_expressions.at(i);
            ErrorReporter::report_type_checker_error(
                current_file->path, expr, NULL, NULL, NULL, "Incorrect name specifier in new expression"
            );
            return;
        }

        if (!type_match(resolved_member_type, expression_type))
        {
            auto [name, expr] = expression->named_expressions.at(i);
            ErrorReporter::report_type_checker_error(
                current_file->path, expression, expr, NULL, NULL, "Mismatched types in new expression"
            );
            return;
        }
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

void TypeChecker::type_check_null_literal_expression(NullLiteralExpression *expression, SymbolTable *symbol_table) {
    expression->type_info = new PointerTypeInfo{TypeInfoType::POINTER, new AnyTypeInfo{TypeInfoType::ANY}};
}

void TypeChecker::type_check_propagation_expression(PropagateExpression *expression, SymbolTable *symbol_table) {
    TRY_CALL(type_check_expression(expression->expression, symbol_table));
    TRY_CALL(type_check_type_expression(expression->type_expression, symbol_table));
    TRY_CALL(type_check_type_expression(expression->otherwise, symbol_table));

    if (expression->expression->type_info->type != TypeInfoType::UNION)
    {
        ErrorReporter::report_type_checker_error(
            current_file->path, expression->expression, NULL, NULL, NULL, "Can only use propagation on union types"
        );
        return;
    }

    // find the index of the type expression in the union type if
    // not there then report an error
    auto expression_union_type = static_cast<UnionTypeInfo *>(expression->expression->type_info);
    s64 index                  = -1;
    for (s64 i = 0; i < expression_union_type->types.size(); i++)
    {
        auto t = expression_union_type->types[i];
        if (type_match(t, expression->type_expression->type_info))
        {
            index = i;
            break;
        }
    }

    if (index == -1)
    {
        ErrorReporter::report_type_checker_error(
            current_file->path, expression->expression, NULL, expression->type_expression, NULL,
            "Union type does not contain this type"
        );
        return;
    }

    // we konw index in union types of our type, if we remove this type the resulting
    // type is the return type, if the union with the type removed is a single type
    // then it is converted to a single type without the union
    auto remaining_types = std::vector<TypeInfo *>();
    for (s64 i = 0; i < expression_union_type->types.size(); i++)
    {
        if (i != index)
        { remaining_types.push_back(expression_union_type->types[i]); }
    }

    if (remaining_types.size() == 1)
    { expression->type_info = remaining_types[0]; }
    else
    { expression->type_info = new UnionTypeInfo{TypeInfoType::UNION, remaining_types}; }

    if (!type_match(expression->type_info, expression->otherwise->type_info))
    {
        ErrorReporter::report_type_checker_error(
            current_file->path, expression->expression, NULL, expression->type_expression, expression->otherwise,
            "Indicated type does not match final type with the given type removed"
        );
        return;
    }
}

void TypeChecker::type_check_zero_literal_expression(ZeroLiteralExpression *expression, SymbolTable *symbol_table) {
    expression->type_info = new AnyTypeInfo{TypeInfoType::ANY};
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

    panic("Internal :: Cannot type check this operator yet...");
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
    auto [type, error] = symbol_table->get_type(&type_expression->identifier);
    if (error)
    {
        ErrorReporter::report_type_checker_error(
            current_file->path, NULL, NULL, type_expression, NULL, "Unrecognised type in type expression"
        );
        return;
    }

    type_expression->type_info = type;
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
        return true; // TODO: this might be a bug n ot checking b but not sure
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

    if (a->type == TypeInfoType::VOID || a->type == TypeInfoType::BOOLEAN || a->type == TypeInfoType::STRING)
    { // values don't matter
        return true;
    }
    else if (a->type == TypeInfoType::NUMBER)
    {
        auto int_a = static_cast<NumberTypeInfo *>(a);
        auto int_b = static_cast<NumberTypeInfo *>(b);

        if (int_a->size == int_b->size && int_a->type == int_b->type)
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

        // symbol table does not copy struct type infos
        // so pointer to one struct instance of a type
        // is ALWAYS the same as another
        return struct_a == struct_b;
    }
    else if (a->type == TypeInfoType::POINTER)
    {
        auto ptr_a = static_cast<PointerTypeInfo *>(a);
        auto ptr_b = static_cast<PointerTypeInfo *>(b);

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
        // unions can get tricky sometimes
        // you can either try and match the unions through each
        // of the members
        // or you may want to use a as a member of the b union
        auto union_a = static_cast<UnionTypeInfo *>(a);
        auto union_b = static_cast<UnionTypeInfo *>(b);

        // of the size matches then try and match all the types of
        // the union if the members of the union do not match then
        // try and match as an instance
        if (union_a->types.size() == union_b->types.size())
        {
            bool match = true;
            for (int i = 0; i < union_a->types.size(); i++)
            {
                if (!type_match(union_a->types.at(i), union_b->types.at(i)))
                {
                    match = false;
                    break;
                }
            }

            if (match)
                return true;
        }

        // if we are here then they are not the same union
        // now try and see if a is a type of b
        for (int i = 0; i < union_b->types.size(); i++)
        {
            if (type_match(union_a, union_b->types.at(i)))
            { return true; }
        }

        return false;
    }
    else if (a->type == TypeInfoType::ENUM)
    {
        auto enum_a = static_cast<EnumTypeInfo *>(a);
        auto enum_b = static_cast<EnumTypeInfo *>(b);

        return enum_a->identifier == enum_b->identifier;
    }

    panic("Cannot type check this type info");
    return false;
}