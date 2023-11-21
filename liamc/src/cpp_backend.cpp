#include "cpp_backend.h"

#include <algorithm>
#include <format>
#include <string>

#include "args.h"
#include "ast.h"

std::string CppBackend::emit(CompilationBundle *bundle) {
    std::string source_generated = "#include <core.h>\n#include<liam_stdlib.h>\n\n";

    source_generated.append("////////////////////////\n//\n// forward declarations\n//\n////////////////////////\n");
    for (CompilationUnit *cu : bundle->compilation_units)
    {
        this->compilation_unit = cu;
        // forward declarations
        for (auto stmt : this->compilation_unit->top_level_struct_statements)
        {
            source_generated.append(forward_declare_struct(stmt));
        }

        for (auto stmt : this->compilation_unit->top_level_fn_statements)
        {
            source_generated.append(forward_declare_function(stmt));
        }
    }

    source_generated.append("\n\n");
    source_generated.append("///////////////////\n//\n// implementations \n//\n///////////////////\n");
    for (CompilationUnit *cu : bundle->compilation_units)
    {
        this->compilation_unit = cu;

        // struct bodys
        for (auto stmt : this->compilation_unit->top_level_struct_statements)
        {
            source_generated.append(emit_struct_statement(stmt));
        }

        // function bodys
        for (auto stmt : this->compilation_unit->top_level_fn_statements)
        {
            source_generated.append(emit_fn_statement(stmt));
        }
    }
    source_generated.append("\nint main(int argc, char **argv) { main::main(); return 0; }\n\n\n // GOODBYE");

    return source_generated;
}

std::string CppBackend::forward_declare_struct(StructStatement *statement) {
    std::string source = "";

    source.append(std::format("namespace {} {{", get_namespace_name(this->compilation_unit)));
    source.append("struct " + this->compilation_unit->get_token_string_from_index(statement->identifier) + ";");
    source.append("}\n");
    return source;
}

std::string CppBackend::forward_declare_function(FnStatement *statement) {
    std::string source = "";
    source.append(std::format("namespace {} {{", get_namespace_name(this->compilation_unit)));
    source.append(emit_type_expression(statement->return_type) + " ");

    std::string name = this->compilation_unit->get_token_string_from_index(statement->identifier);
    source.append(name);
    source.append("(");

    u64 index = 0;
    for (auto [token_index, type] : statement->params)
    {
        std::string identifier_string = this->compilation_unit->get_token_string_from_index(token_index);
        source.append(emit_type_expression(type) + " " + identifier_string);
        if (index + 1 < statement->params.size())
        {
            source.append(", ");
        }
        index++;
    }
    source.append(");");
    source.append("}\n");
    return source;
}

std::string CppBackend::emit_statement(Statement *statement) {
    switch (statement->statement_type)
    {
    case StatementType::STATEMENT_RETURN:
        return emit_return_statement(static_cast<ReturnStatement *>(statement));
        break;
    case StatementType::STATEMENT_BREAK:
        return emit_break_statement(static_cast<BreakStatement *>(statement));
        break;
    case StatementType::STATEMENT_LET:
        return emit_let_statement(static_cast<LetStatement *>(statement));
        break;
    case StatementType::STATEMENT_FN:
        return emit_fn_statement(static_cast<FnStatement *>(statement));
        break;
    case StatementType::STATEMENT_SCOPE:
        return emit_scope_statement(static_cast<ScopeStatement *>(statement));
        break;
    case StatementType::STATEMENT_STRUCT:
        return emit_struct_statement(static_cast<StructStatement *>(statement));
        break;
    case StatementType::STATEMENT_ASSIGNMENT:
        return emit_assigment_statement(static_cast<AssigmentStatement *>(statement));
        break;
    case StatementType::STATEMENT_EXPRESSION:
        return emit_expression_statement(static_cast<ExpressionStatement *>(statement));
        break;
    case StatementType::STATEMENT_FOR:
        return emit_for_statement(static_cast<ForStatement *>(statement));
        break;
    case StatementType::STATEMENT_IF:
        return emit_if_statement(static_cast<IfStatement *>(statement));
        break;
    case StatementType::STATEMENT_CONTINUE:
        return emit_continue_statement(static_cast<ContinueStatement *>(statement));
        break;
    }

    UNREACHABLE();
}

std::string CppBackend::emit_return_statement(ReturnStatement *statement) {
    std::string source = "return ";
    if (statement->expression)
    {
        source.append(emit_expression(statement->expression));
    }
    return source + ";";
}

std::string CppBackend::emit_break_statement(BreakStatement *statement) {
    return "break;";
}

std::string CppBackend::emit_let_statement(LetStatement *statement) {
    auto source = std::string();
    if (statement->type)
    {
        source.append(emit_type_expression(statement->type) + " ");
    }
    else
    {
        source.append("auto ");
    }
    source.append(this->compilation_unit->get_token_string_from_index(statement->identifier));
    source.append(" = ");
    source.append(emit_expression(statement->rhs) + ";\n");
    return source;
}

std::string CppBackend::emit_scope_statement(ScopeStatement *statement) {
    auto source = std::string();
    source.append("{\n");
    for (auto stmt : statement->statements)
    {
        source.append(emit_statement(stmt));
    }
    source.append("}");
    return source;
}

std::string CppBackend::emit_fn_statement(FnStatement *statement) {
    auto source = std::string();

    source.append(std::format("namespace {} {{\n", get_namespace_name(this->compilation_unit)));
    source.append(emit_type_expression(statement->return_type) + " ");

    std::string name = this->compilation_unit->get_token_string_from_index(statement->identifier);

    source.append(name);
    source.append("(");

    u64 index = 0;
    for (auto [identifier, type] : statement->params)
    {
        std::string identifier_string = this->compilation_unit->get_token_string_from_index(identifier);
        source.append(emit_type_expression(type) + " " + identifier_string);
        if (index + 1 < statement->params.size())
        {
            source.append(", ");
        }
        index++;
    }

    source.append(")");
    source.append(emit_scope_statement(statement->body));
    source.append("\n}\n");
    return source;
}

std::string CppBackend::emit_struct_statement(StructStatement *statement) {
    auto source = std::string();

    source.append(std::format("namespace {} {{\n", get_namespace_name(this->compilation_unit)));
    source.append("struct " + this->compilation_unit->get_token_string_from_index(statement->identifier) + " {");
    // members
    for (auto [identifier_token_index, type] : statement->members)
    {
        source.append("\n" + emit_type_expression(type) + " ");
        source.append(this->compilation_unit->get_token_string_from_index(identifier_token_index) + ";");
    }

    source.append("};\n}\n"); // extra } for the namespace closing
    return source;
}

std::string CppBackend::emit_assigment_statement(AssigmentStatement *statement) {
    auto source = std::string();

    source.append(emit_expression(statement->lhs));
    source.append(" = ");
    source.append(emit_expression(statement->assigned_to->expression));

    source.append(";\n");
    return source;
}

std::string CppBackend::emit_expression_statement(ExpressionStatement *statement) {
    return emit_expression(statement->expression) + ";\n";
}

std::string CppBackend::emit_for_statement(ForStatement *statement) {
    std::string source = "";
    source.append(
        "for(" + emit_statement(statement->assign) + emit_expression(statement->condition) + ";" +
        strip_semi_colon(emit_statement(statement->update)) + ")"
    );
    source.append(emit_scope_statement(statement->body));
    return source;
}

std::string CppBackend::emit_if_statement(IfStatement *statement) {
    std::string source = "";

    source.append("if (");
    source.append(emit_expression(statement->expression));
    source.append(") ");
    source.append(emit_scope_statement(statement->body));

    if (statement->else_statement)
    {
        source.append(emit_else_statement(statement->else_statement));
    }

    return source;
}

std::string CppBackend::emit_else_statement(ElseStatement *statement) {
    std::string source = "else ";

    if (statement->if_statement)
    {
        source.append(emit_if_statement(statement->if_statement));
    }
    else if (statement->body)
    {
        source.append(emit_scope_statement(statement->body));
    }
    return source;
}

std::string CppBackend::emit_continue_statement(ContinueStatement *statement) {
    return "continue;";
}

std::string CppBackend::emit_expression(Expression *expression) {
    switch (expression->type)
    {
    case ExpressionType::EXPRESSION_STRING_LITERAL:
        return emit_string_literal_expression(static_cast<StringLiteralExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_NUMBER_LITERAL:
        return emit_int_literal_expression(static_cast<NumberLiteralExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_BOOL_LITERAL:
        return emit_bool_literal_expression(static_cast<BoolLiteralExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_CALL:
        return emit_call_expression(static_cast<CallExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_IDENTIFIER:
        return emit_identifier_expression(static_cast<IdentifierExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_BINARY:
        return emit_binary_expression(static_cast<BinaryExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_UNARY:
        return emit_unary_expression(static_cast<UnaryExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_GET:
        return emit_get_expression(static_cast<GetExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_GROUP:
        return emit_group_expression(static_cast<GroupExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_NULL_LITERAL:
        return emit_null_literal_expression(static_cast<NullLiteralExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_ZERO_LITERAL:
        return emit_zero_literal_expression(static_cast<ZeroLiteralExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_INSTANTIATION:
        return emit_instantiate_expression(static_cast<InstantiateExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_STRUCT_INSTANCE:
        return emit_struct_instance_expression(static_cast<StructInstanceExpression *>(expression));
        break;
    default: {
        panic("Cannot emit this expression in cpp backend");
        return "";
    }
    }
}

std::string CppBackend::emit_binary_expression(BinaryExpression *expression) {
    auto source = std::string();
    source.append(emit_expression(expression->left));
    switch (expression->op)
    {
    case TokenType::TOKEN_PLUS:
        source.append(" + ");
        break;
    case TokenType::TOKEN_MINUS:
        source.append(" - ");
        break;
    case TokenType::TOKEN_STAR:
        source.append(" * ");
        break;
    case TokenType::TOKEN_SLASH:
        source.append(" / ");
        break;
    case TokenType::TOKEN_MOD:
        source.append(" % ");
        break;
    case TokenType::TOKEN_OR:
        source.append(" || ");
        break;
    case TokenType::TOKEN_AND:
        source.append(" && ");
        break;
    case TokenType::TOKEN_EQUAL:
        source.append(" == ");
        break;
    case TokenType::TOKEN_NOT_EQUAL:
        source.append(" != ");
        break;
    case TokenType::TOKEN_LESS:
        source.append(" < ");
        break;
    case TokenType::TOKEN_GREATER:
        source.append(" > ");
        break;
    case TokenType::TOKEN_GREATER_EQUAL:
        source.append(" >= ");
        break;
    case TokenType::TOKEN_LESS_EQUAL:
        source.append(" <= ");
        break;
    default:
        panic("Cannot use this operand in the cpp backend");
        break;
    }
    source.append(emit_expression(expression->right));

    return source;
}

std::string CppBackend::emit_string_literal_expression(StringLiteralExpression *expression) {
    std::string literal_string = this->compilation_unit->get_token_string_from_index(expression->token);
    return "LiamInternal::make_str((char*)" + literal_string + ", " +
           std::to_string(string_literal_length(&literal_string)) + ")";
}

std::string CppBackend::emit_bool_literal_expression(BoolLiteralExpression *expression) {
    return this->compilation_unit->get_token_string_from_index(expression->token);
}

std::string CppBackend::emit_int_literal_expression(NumberLiteralExpression *expression) {
    auto number_type = static_cast<NumberTypeInfo *>(expression->type_info);

    std::string func_call = "LiamInternal::__";

    if (number_type->number_type == NumberType::UNSIGNED)
    {
        func_call.append("u");
    }
    else if (number_type->number_type == NumberType::SIGNED)
    {
        func_call.append("i");
    }
    else if (number_type->number_type == NumberType::FLOAT)
    {
        func_call.append("f");
    }

    if (number_type->size == 8)
    {
        func_call.append("8");
    }
    else if (number_type->size == 16)
    {
        func_call.append("16");
    }
    else if (number_type->size == 32)
    {
        func_call.append("32");
    }
    else if (number_type->size == 64)
    {
        func_call.append("64");
    }

    func_call.append("(" + std::to_string(expression->number) + ")");

    return func_call;
}

std::string CppBackend::emit_unary_expression(UnaryExpression *expression) {
    if (expression->op == TokenType::TOKEN_AMPERSAND)
    {
        return "&(" + emit_expression(expression->expression) + ")";
    }
    else if (expression->op == TokenType::TOKEN_STAR)
    {
        return "*(" + emit_expression(expression->expression) + ")";
    }
    else if (expression->op == TokenType::TOKEN_NOT)
    {
        return "!(" + emit_expression(expression->expression) + ")";
    }

    panic("Got a unrecognized operand in cpp backend");
    return "";
}

std::string CppBackend::emit_call_expression(CallExpression *expression) {
    auto source = std::string();
    source.append(emit_expression(expression->callee));

    source.append("(");

    if (expression->callee->type == ExpressionType::EXPRESSION_GET)
    {
        auto get_expression = static_cast<GetExpression *>(expression->callee);

        // because we can call members of a type T on ^T sometimes we do not need to add
        // a & as it is already a pointer, if it is not a pointer then add the &
        if (get_expression->lhs->type_info->type != TypeInfoType::POINTER)
        {
            source.append("&");
        }

        source.append(emit_expression(get_expression->lhs));
        if (expression->args.size() > 0)
        {
            source.append(", ");
        }
    }

    u64 index = 0;
    for (auto expr : expression->args)
    {
        source.append(emit_expression(expr));

        if (index + 1 < expression->args.size())
        {
            source.append(", ");
        }
        index++;
    }
    source.append(")");
    return source;
}

std::string CppBackend::emit_identifier_expression(IdentifierExpression *expression) {
    return this->compilation_unit->get_token_string_from_index(expression->identifier);
}

std::string CppBackend::emit_get_expression(GetExpression *expression) {

    // TODO: verify this code makes sense I am re-reading it and I dont know how it works
    // to stop conflicts with members functions we emit __func for the get expression

    std::string member_string = this->compilation_unit->get_token_string_from_index(expression->member);

    if (expression->type_info->type == TypeInfoType::FN)
    {
        return "__" + member_string;
    }

    if (expression->lhs->type_info->type == TypeInfoType::POINTER)
    {
        return emit_expression(expression->lhs) + "->" + member_string;
    }

    return emit_expression(expression->lhs) + "." + member_string;
}

std::string CppBackend::emit_group_expression(GroupExpression *expression) {
    return "(" + emit_expression(expression->expression) + ")";
}

std::string CppBackend::emit_null_literal_expression(NullLiteralExpression *expression) {
    return "nullptr";
}

std::string CppBackend::emit_zero_literal_expression(ZeroLiteralExpression *expression) {
    return "{}";
}

std::string CppBackend::emit_instantiate_expression(InstantiateExpression *expression) {
    auto source = std::string();
    source.append(emit_expression(expression->expression));
    return source;
}

std::string CppBackend::emit_struct_instance_expression(StructInstanceExpression *expression) {
    std::string source;

    std::string identifier_string = emit_type_expression(expression->type_expression);
    source.append(identifier_string);

    source.append("{");
    u64 index = 0;
    for (auto [name, expr] : expression->named_expressions)
    {
        source.append(emit_expression(expr));
        if (index + 1 < expression->named_expressions.size())
        {
            source.append(", ");
        }
        index++;
    }
    source.append("}");

    return source;
}

std::string CppBackend::emit_type_expression(TypeExpression *type_expression) {
    switch (type_expression->type)
    {
    case TypeExpressionType::TYPE_IDENTIFIER:
        return emit_identifier_type_expression(static_cast<IdentifierTypeExpression *>(type_expression));
        break;
    case TypeExpressionType::TYPE_UNARY:
        return emit_unary_type_expression(static_cast<UnaryTypeExpression *>(type_expression));
        break;
    case TypeExpressionType::TYPE_GET:
        return emit_get_type_expression(static_cast<GetTypeExpression *>(type_expression));
    default:
        panic("Cpp back end does not support this type expression");
        return "";
    }
}

std::string CppBackend::emit_unary_type_expression(UnaryTypeExpression *type_expression) {

    if (type_expression->unary_type == UnaryType::POINTER)
    {
        return emit_type_expression(type_expression->type_expression) + "*";
    }

    panic("Cpp backend does not support this op yet...");
    return "";
}

std::string CppBackend::emit_identifier_type_expression(IdentifierTypeExpression *type_expression) {
    return this->compilation_unit->get_token_string_from_index(type_expression->identifier);
}

std::string CppBackend::emit_get_type_expression(GetTypeExpression *type_expression) {
    return "THIS IS A GET TYPE EXPRESSION.";
}

std::string strip_semi_colon(std::string str) {
    if (str.size() == 0)
        return str;

    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());

    if (str.back() == ';')
    {
        return str.substr(0, str.size() - 1);
    }
    else
    {
        return str;
    }
    // return (str[str.size() - 1] == ';') ? str.substr(0, str.size()-1) : str;
}

u64 string_literal_length(std::string *string) {
    u64 length = 0;

    for (u64 i = 0; i < string->size(); i++)
    {
        if (string->at(i) == '\\' && i + 1 < string->size())
        {
            i += 2;
        }

        length++;
    }

    return length;
}

std::string get_namespace_name(CompilationUnit *compilation_unit) {
    return compilation_unit->file_data->absolute_path.stem();
}
