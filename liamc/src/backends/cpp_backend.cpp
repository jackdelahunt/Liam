#include "cpp_backend.h"

#include "fmt/core.h"
#include <algorithm>
#include <string>

std::string CppBackend::emit(File *file) {
    auto source_generated = std::string("#include \"lib.h\"\n\n");

    source_generated.append("// enum forward declarations\n");
    for (auto stmt : file->statements)
    {
        if (stmt->statement_type == StatementType::STATEMENT_ENUM)
        { source_generated.append(forward_declare_enum(static_cast<EnumStatement *>(stmt))); }
    }

    source_generated.append("// struct forward declarations\n");
    for (auto stmt : file->statements)
    {
        if (stmt->statement_type == StatementType::STATEMENT_STRUCT)
        { source_generated.append(forward_declare_struct(static_cast<StructStatement *>(stmt))); }
    }

    source_generated.append("\n// function forward declarations\n");

    for (auto stmt : file->statements)
    {
        if (stmt->statement_type == StatementType::STATEMENT_FN)
        { source_generated.append(forward_declare_function(static_cast<FnStatement *>(stmt))); }
    }

    source_generated.append("\n// Source\n");

    for (auto stmt : file->statements)
    { source_generated.append(emit_statement(stmt)); }

    source_generated.append("int main(int argc, char **argv) { __liam__main__(); }");

    return source_generated;
}

std::string CppBackend::forward_declare_enum(EnumStatement *statement) {
    return "enum class " + statement->identifier.string + ";\n";
}

std::string CppBackend::forward_declare_struct(StructStatement *statement) {
    if (statement->is_extern)
        return "";

    std::string source = "";

    source.append(emit_cpp_template_declaration(&statement->generics));
    source.append("struct " + statement->identifier.string + ";\n");

    return source;
}

std::string CppBackend::forward_declare_function(FnStatement *statement) {
    if (statement->is_extern)
        return "";

    std::string source = "";

    source.append(emit_cpp_template_declaration(&statement->generics));
    source.append(emit_type_expression(statement->return_type) + " ");

    if (statement->identifier.string == "main")
    { source.append(" __liam__main__"); }
    else
    { source.append(statement->identifier.string); }

    source.append("(");
    int index = 0;
    for (auto [identifier, type] : statement->params)
    {
        source.append(emit_type_expression(type) + " " + identifier.string);
        if (index + 1 < statement->params.size())
        { source.append(", "); }
        index++;
    }
    source.append(");\n");
    return source;
}

std::string CppBackend::emit_statement(Statement *statement) {
    switch (statement->statement_type)
    {
    case StatementType::STATEMENT_INSERT:
        return emit_insert_statement(dynamic_cast<InsertStatement *>(statement));
        break;
    case StatementType::STATEMENT_RETURN:
        return emit_return_statement(dynamic_cast<ReturnStatement *>(statement));
        break;
    case StatementType::STATEMENT_BREAK:
        return emit_break_statement(dynamic_cast<BreakStatement *>(statement));
        break;
    case StatementType::STATEMENT_LET:
        return emit_let_statement(dynamic_cast<LetStatement *>(statement));
        break;
    case StatementType::STATEMENT_FN:
        return emit_fn_statement(dynamic_cast<FnStatement *>(statement));
        break;
    case StatementType::STATEMENT_SCOPE:
        return emit_scope_statement(dynamic_cast<ScopeStatement *>(statement));
        break;
    case StatementType::STATEMENT_STRUCT:
        return emit_struct_statement(dynamic_cast<StructStatement *>(statement));
        break;
    case StatementType::STATEMENT_ASSIGNMENT:
        return emit_assigment_statement(dynamic_cast<AssigmentStatement *>(statement));
        break;
    case StatementType::STATEMENT_EXPRESSION:
        return emit_expression_statement(dynamic_cast<ExpressionStatement *>(statement));
        break;
    case StatementType::STATEMENT_FOR:
        return emit_for_statement(dynamic_cast<ForStatement *>(statement));
        break;
    case StatementType::STATEMENT_IF:
        return emit_if_statement(dynamic_cast<IfStatement *>(statement));
        break;
    case StatementType::STATEMENT_ENUM:
        return emit_enum_statement(dynamic_cast<EnumStatement *>(statement));
        break;
    }

    panic("Statement not implemented in c back end :[");
    return "";
}

std::string CppBackend::emit_insert_statement(InsertStatement *statement) {
    if (statement->byte_code->type != ExpressionType::EXPRESSION_STRING_LITERAL)
    { panic("Cannot emit non string literal in insert"); }

    auto string_literal = dynamic_cast<StringLiteralExpression *>(statement->byte_code);
    return string_literal->token.string;
}

std::string CppBackend::emit_return_statement(ReturnStatement *statement) {
    return "return " + emit_expression(statement->expression) + ";";
}

std::string CppBackend::emit_break_statement(BreakStatement *statement) {
    panic("Not implemented in C backend");
    return "";
}

std::string CppBackend::emit_let_statement(LetStatement *statement) {
    auto source = std::string();
    if (statement->type)
    { source.append(emit_type_expression(statement->type) + " "); }
    else
    { source.append("auto "); }
    source.append(statement->identifier.string);
    source.append(" = ");
    source.append(emit_expression(statement->rhs) + ";\n");
    return source;
}

std::string CppBackend::emit_scope_statement(ScopeStatement *statement) {
    auto source = std::string();
    source.append("{\n");
    for (auto stmt : statement->statements)
    { source.append(emit_statement(stmt)); }
    source.append("\n}\n\n");
    return source;
}

std::string CppBackend::emit_fn_statement(FnStatement *statement) {
    if (statement->is_extern)
        return "";

    auto source = std::string();
    if (statement->generics.size() > 0)
    {
        int index = 0;
        source.append("template <");
        for (auto &generic : statement->generics)
        {
            source.append("typename " + generic.string);
            if (index + 1 < statement->generics.size())
            { source.append(", "); }
            index++;
        }
        source.append(">\n");
    }

    source.append(emit_type_expression(statement->return_type) + " ");

    if (statement->identifier.string == "main")
    { source.append("__liam__main__"); }
    else
    { source.append(statement->identifier.string); }

    source.append("(");
    int index = 0;
    for (auto [identifier, type] : statement->params)
    {
        source.append(emit_type_expression(type) + " " + identifier.string);
        if (index + 1 < statement->params.size())
        { source.append(", "); }
        index++;
    }
    source.append(")");
    source.append(emit_scope_statement(statement->body));
    return source;
}

std::string CppBackend::emit_struct_statement(StructStatement *statement) {
    if (statement->is_extern)
        return "";
    auto source = std::string();

    source.append(emit_cpp_template_declaration(&statement->generics));

    source.append("struct " + statement->identifier.string + " {");
    // members
    for (auto [identifier, type] : statement->members)
    {
        source.append("\n" + emit_type_expression(type) + " ");
        source.append(identifier.string + ";");
    }

    source.append("};\n");

    // pretty print
    source.append(emit_cpp_template_declaration(&statement->generics));
    source.append(
        "std::ostream& operator<<(std::ostream& os, const " + statement->identifier.string +
        emit_cpp_template_usage(&statement->generics) + " &obj) {\n"
    );

    // header
    source.append("os << \"" + statement->identifier.string + "\" << \" {\" << std::endl;\n");

    // body
    for (auto [identifier, type] : statement->members)
    {
        if (type->type_info->type == TypeInfoType::POINTER)
        {
            source.append(
                "os << \"   \" << \"*" + identifier.string + ": \" << *(obj." + identifier.string +
                ") << \",\" << std::endl;\n"
            );
        }
        else
        {
            source.append(
                "os << \"   \" << \"" + identifier.string + ": \" << obj." + identifier.string +
                " << \",\" << std::endl;\n"
            );
        }
    }

    // tail
    source.append("os << \"}\" << std::endl;\n");

    source.append("return os;\n");

    source.append("}\n\n");
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
        "for(" + emit_let_statement(statement->let_statement) + emit_expression(statement->condition) + ";" +
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
    { source.append(emit_else_statement(statement->else_statement)); }

    return source;
}

std::string CppBackend::emit_else_statement(ElseStatement *statement) {
    std::string source = "else ";

    if (statement->if_statement)
    { source.append(emit_if_statement(statement->if_statement)); }
    else if (statement->body)
    { source.append(emit_scope_statement(statement->body)); }
    return source;
}

std::string CppBackend::emit_enum_statement(EnumStatement *statement) {
    std::string source = "enum class ";

    source.append(statement->identifier.string);
    source.append(" {\n");

    for (auto &instance : statement->instances)
    { source.append(instance.string + ",\n"); }

    source.append(" };\n");

    // pretty print
    source.append("std::ostream& operator<<(std::ostream& os, const " + statement->identifier.string + " &obj) {\n");

    source.append("switch(obj) {\n");
    for (auto &t : statement->instances)
    {
        source.append(fmt::format(
            "case {}::{}: os << \"{}.{}\"; break;\n", statement->identifier.string, t.string,
            statement->identifier.string, t.string
        ));
    }
    source.append("}\n");

    source.append("return os;\n");

    source.append("}\n\n");

    return source;
}

std::string CppBackend::emit_expression(Expression *expression) {
    switch (expression->type)
    {
    case ExpressionType::EXPRESSION_STRING_LITERAL:
        return emit_string_literal_expression(dynamic_cast<StringLiteralExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_NUMBER_LITERAL:
        return emit_int_literal_expression(dynamic_cast<NumberLiteralExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_BOOL_LITERAL:
        return emit_bool_literal_expression(dynamic_cast<BoolLiteralExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_CALL:
        return emit_call_expression(dynamic_cast<CallExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_IDENTIFIER:
        return emit_identifier_expression(dynamic_cast<IdentifierExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_BINARY:
        return emit_binary_expression(dynamic_cast<BinaryExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_IS:
        return emit_is_expression(dynamic_cast<IsExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_UNARY:
        return emit_unary_expression(dynamic_cast<UnaryExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_GET:
        return emit_get_expression(dynamic_cast<GetExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_NEW:
        return emit_new_expression(dynamic_cast<NewExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_GROUP:
        return emit_group_expression(dynamic_cast<GroupExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_NULL_LITERAL:
        return emit_null_literal_expression(dynamic_cast<NullLiteralExpression *>(expression));
        break;
    default: {
        panic("Cannot emit this expression in cpp backend");
        return "";
    }
    }
}

std::string CppBackend::emit_is_expression(IsExpression *expression) {
    auto source = std::string();

    source.append("auto " + expression->identifier.string + " = ");
    source.append("std::get_if<" + emit_type_expression(expression->type_expression) + ">(");
    source.append("&" + emit_expression(expression->expression));
    source.append(")");

    return source;
}

std::string CppBackend::emit_binary_expression(BinaryExpression *expression) {
    auto source = std::string();
    source.append(emit_expression(expression->left));
    switch (expression->op.type)
    {
    case TOKEN_PLUS:
        source.append(" + ");
        break;
    case TOKEN_MINUS:
        source.append(" - ");
        break;
    case TOKEN_STAR:
        source.append(" * ");
        break;
    case TOKEN_SLASH:
        source.append(" / ");
        break;
    case TOKEN_MOD:
        source.append(" % ");
        break;
    case TOKEN_OR:
        source.append(" || ");
        break;
    case TOKEN_AND:
        source.append(" && ");
        break;
    case TOKEN_EQUAL:
        source.append(" == ");
        break;
    case TOKEN_NOT_EQUAL:
        source.append(" != ");
        break;
    case TOKEN_LESS:
        source.append(" < ");
        break;
    case TOKEN_GREATER:
        source.append(" > ");
        break;
    case TOKEN_GREATER_EQUAL:
        source.append(" >= ");
        break;
    case TOKEN_LESS_EQUAL:
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
    // -2 is for the quotes size
    return "make_str((char*)" + expression->token.string + ", " + std::to_string(expression->token.string.size() - 2) +
           ")";
}

std::string CppBackend::emit_bool_literal_expression(BoolLiteralExpression *expression) {
    return expression->value.string;
}

std::string CppBackend::emit_int_literal_expression(NumberLiteralExpression *expression) {
    auto number_type = static_cast<NumberTypeInfo *>(expression->type_info);

    std::string func_call = "_";

    if (number_type->type == UNSIGNED)
    { func_call.append("u"); }
    else if (number_type->type == SIGNED)
    { func_call.append("s"); }
    else if (number_type->type == FLOAT)
    { func_call.append("f"); }

    if (number_type->size == 8)
    { func_call.append("8"); }
    else if (number_type->size == 16)
    { func_call.append("16"); }
    else if (number_type->size == 32)
    { func_call.append("32"); }
    else if (number_type->size == 64)
    { func_call.append("64"); }

    func_call.append("(" + std::to_string(expression->number) + ")");

    return func_call;
}

std::string CppBackend::emit_unary_expression(UnaryExpression *expression) {
    if (expression->op.type == TOKEN_AT)
    { return "&(" + emit_expression(expression->expression) + ")"; }
    else if (expression->op.type == TOKEN_STAR)
    { return "*(" + emit_expression(expression->expression) + ")"; }
    else if (expression->op.type == TOKEN_NOT)
    { return "!(" + emit_expression(expression->expression) + ")"; }

    panic("Got a unrecognized operand in cpp backend");
    return "";
}

std::string CppBackend::emit_call_expression(CallExpression *expression) {
    auto source = std::string();
    source.append(emit_expression(expression->identifier));

    if (expression->generics.size() > 0)
    {
        int index = 0;
        source.append("<");
        for (auto type : expression->generics)
        {
            source.append(emit_type_expression(type));

            if (index + 1 < expression->generics.size())
            { source.append(", "); }
            index++;
        }
        source.append(">");
    }

    int index = 0;
    source.append("(");
    for (auto expr : expression->args)
    {
        source.append(emit_expression(expr));

        if (index + 1 < expression->args.size())
        { source.append(", "); }
        index++;
    }
    source.append(")");
    return source;
}

std::string CppBackend::emit_identifier_expression(IdentifierExpression *expression) {
    return expression->identifier.string;
}

std::string CppBackend::emit_get_expression(GetExpression *expression) {
    if (expression->lhs->type_info->type == TypeInfoType::POINTER)
    { return emit_expression(expression->lhs) + "->" + expression->member.string; }

    if (expression->lhs->type_info->type == TypeInfoType::ENUM)
    { return emit_expression(expression->lhs) + "::" + expression->member.string; }

    return emit_expression(expression->lhs) + "." + expression->member.string;
}

std::string CppBackend::emit_new_expression(NewExpression *expression) {
    auto source = std::string();
    source.append(expression->identifier.string);

    source.append(emit_cpp_template_params(&expression->generics));

    source.append("{");
    int index = 0;
    for (auto [name, expr] : expression->named_expressions)
    {
        source.append(emit_expression(expr));
        if (index + 1 < expression->named_expressions.size())
        { source.append(", "); }
        index++;
    }
    source.append("}");

    return source;
}

std::string CppBackend::emit_group_expression(GroupExpression *expression) {
    return "(" + emit_expression(expression->expression) + ")";
}

std::string CppBackend::emit_null_literal_expression(NullLiteralExpression *expression) {
    return "nullptr";
}

std::string CppBackend::emit_type_expression(TypeExpression *type_expression) {
    switch (type_expression->type)
    {
    case TypeExpressionType::TYPE_UNION:
        return emit_union_type_expression(dynamic_cast<UnionTypeExpression *>(type_expression));
        break;
    case TypeExpressionType::TYPE_IDENTIFIER:
        return emit_identifier_type_expression(dynamic_cast<IdentifierTypeExpression *>(type_expression));
        break;
    case TypeExpressionType::TYPE_UNARY:
        return emit_unary_type_expression(dynamic_cast<UnaryTypeExpression *>(type_expression));
        break;
    case TypeExpressionType::TYPE_SPECIFIED_GENERICS:
        return emit_specified_generics_type_expression(dynamic_cast<SpecifiedGenericsTypeExpression *>(type_expression)
        );
        break;
    default:
        panic("Cpp back end does not support this return_type expression");
        return "";
    }
}

std::string CppBackend::emit_union_type_expression(UnionTypeExpression *type_expression) {
    std::string source = "std::variant";
    source.append(emit_cpp_template_params(&type_expression->type_expressions));
    return source;
}

std::string CppBackend::emit_unary_type_expression(UnaryTypeExpression *type_expression) {
    if (type_expression->op.type == TokenType::TOKEN_HAT)
    { return emit_type_expression(type_expression->type_expression) + "*"; }

    panic("Cpp backend does not support this op yet...");
    return "";
}

std::string CppBackend::emit_specified_generics_type_expression(SpecifiedGenericsTypeExpression *type_expression) {
    auto source = emit_identifier_type_expression(type_expression->struct_type);
    if (type_expression->generics.size() > 0)
    {
        int index = 0;
        source.append("<");
        for (auto type : type_expression->generics)
        {
            source.append(emit_type_expression(type));

            if (index + 1 < type_expression->generics.size())
            { source.append(", "); }
            index++;
        }
        source.append(">");
    }

    return source;
}

std::string CppBackend::emit_identifier_type_expression(IdentifierTypeExpression *type_expression) {
    return type_expression->identifier.string;
}

std::string strip_semi_colon(std::string str) {
    if (str.size() == 0)
        return str;

    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());

    if (str.back() == ';')
    { return str.substr(0, str.size() - 1); }
    else
    { return str; }
    // return (str[str.size() - 1] == ';') ? str.substr(0, str.size()-1) : str;
}

// template <typename T,  typename E, typename H>
std::string CppBackend::emit_cpp_template_declaration(std::vector<Token> *generics) {
    std::string source = "";

    if (generics->size() > 0)
    {
        int index = 0;
        source.append("template <");
        for (auto &generic : *generics)
        {
            source.append("typename " + generic.string);
            if (index + 1 < generics->size())
            { source.append(", "); }
            index++;
        }
        source.append(">\n");
    }

    return source;
}

// <T, E, H>
std::string CppBackend::emit_cpp_template_usage(std::vector<Token> *generics) {
    std::string source = "";

    if (generics->size() > 0)
    {
        int index = 0;
        source.append("<");
        for (auto &generic : *generics)
        {
            source.append(generic.string);
            if (index + 1 < generics->size())
            { source.append(", "); }
            index++;
        }
        source.append(">");
    }

    return source;
}

// <boolean, String, u64>
std::string CppBackend::emit_cpp_template_params(std::vector<TypeExpression *> *generics) {
    std::string source = "";

    if (generics->size() > 0)
    {
        int index = 0;
        source.append("<");
        for (auto type : *generics)
        {
            source.append(emit_type_expression(type));

            if (index + 1 < generics->size())
            { source.append(", "); }
            index++;
        }
        source.append(">");
    }

    return source;
}