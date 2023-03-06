#include "cpp_backend.h"

#include "args.h"
#include "fmt/core.h"
#include <algorithm>
#include <string>

std::string CppBackend::emit(std::vector<File *> *files) {

    auto enums   = std::vector<EnumStatement *>();
    auto structs = std::vector<StructStatement *>();
    auto aliases = std::vector<AliasStatement *>();
    auto fns     = std::vector<FnStatement *>();
    auto others  = std::vector<Statement *>();

    for (auto file : *files)
    {
        for (auto stmt : file->statements)
        {
            if (stmt->statement_type == StatementType::STATEMENT_ENUM)
            { enums.push_back(dynamic_cast<EnumStatement *>(stmt)); }
            else if (stmt->statement_type == StatementType::STATEMENT_STRUCT)
            { structs.push_back(dynamic_cast<StructStatement *>(stmt)); }
            else if (stmt->statement_type == StatementType::STATEMENT_ALIAS)
            { aliases.push_back(dynamic_cast<AliasStatement *>(stmt)); }
            else if (stmt->statement_type == StatementType::STATEMENT_FN)
            { fns.push_back(dynamic_cast<FnStatement *>(stmt)); }
            else
            { others.push_back(stmt); }
        }
    }

    auto source_generated = std::string("#include \"lib.h\"\n\n");

    source_generated.append("// enum forward declarations\n");
    for (auto stmt : enums)
    { source_generated.append(forward_declare_enum(stmt)); }

    source_generated.append("// struct forward declarations\n");
    for (auto stmt : structs)
    { source_generated.append(forward_declare_struct(stmt)); }

    source_generated.append("// typedefs\n");
    for (auto stmt : aliases)
    { source_generated.append(emit_alias_statement(stmt)); }

    source_generated.append("\n// function forward declarations\n");
    for (auto stmt : fns)
    { source_generated.append(forward_declare_function(stmt)); }

    source_generated.append("\n// Source\n");
    // enum body decls
    for (auto stmt : enums)
    { source_generated.append(emit_enum_statement(stmt)); }

    // struct body decls
    for (auto stmt : structs)
    { source_generated.append(emit_struct_statement(stmt)); }

    // fn body decls
    for (auto stmt : fns)
    { source_generated.append(emit_fn_statement(stmt)); }

    // anything else
    for (auto stmt : others)
    { source_generated.append(emit_statement(stmt)); }

    source_generated.append("int main(int argc, char **argv) { __liam__main__(); return 0; }");

    return source_generated;
}

std::string CppBackend::forward_declare_enum(EnumStatement *statement) {
    std::string source = "";

    // enum struct member namespace forward decl
    source.append("namespace __" + statement->identifier.string + "Members {\n");

    for (auto &member : statement->members)
    { source.append("struct " + member.identifier.string + ";\n"); }

    source.append("}\n\n");

    // enum (we use struct) forward decl
    source.append("struct " + statement->identifier.string + ";\n");
    return source;
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
    else if (statement->parent_type != NULL)
    {
        // add a __ to any member functions, this keeps the inclusion that they are scoped
        // to the type even though they are not in the cpp generation
        source.append("__" + statement->identifier.string);
    }
    else
    { source.append(statement->identifier.string); }

    source.append("(");

    if (statement->parent_type != NULL)
    {
        source.append(emit_type_expression(statement->parent_type) + " *self");
        if (statement->params.size() > 0)
        { source.append(", "); }
    }

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
    case StatementType::STATEMENT_CONTINUE:
        return emit_continue_statement(dynamic_cast<ContinueStatement *>(statement));
        break;
    case StatementType::STATEMENT_ALIAS:
        return emit_alias_statement(dynamic_cast<AliasStatement *>(statement));
        break;
    }

    panic("Statement not implemented in cpp back end :[");
    return "";
}

std::string CppBackend::emit_return_statement(ReturnStatement *statement) {
    std::string source = "return ";
    if (statement->expression)
    { source.append(emit_expression(statement->expression)); }
    return source + ";";
}

std::string CppBackend::emit_break_statement(BreakStatement *statement) {
    return "break;";
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
    else if (statement->parent_type != NULL)
    {
        // add a __ to any member functions, this keeps the illusion that they are scoped
        // to the type even though they are not in the cpp generation
        source.append("__" + statement->identifier.string);
    }
    else
    { source.append(statement->identifier.string); }

    source.append("(");
    int index = 0;

    // if this is a member function add the magic *self parameter
    if (statement->parent_type != NULL)
    {
        source.append(emit_type_expression(statement->parent_type) + " *self");
        if (statement->params.size() > 0)
        { source.append(", "); }
    }

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
    std::string source = "";

    /*
     * namespace __{EnumName}Members {
     *      struct Member {
     *          type __1;
     *          type __2;
     *      };
     * }
     */

    // emit the enums namespace for the members it has
    // then go on to generate the enum which will
    // have its members as the structs defined in the namespace
    source.append("namespace __" + statement->identifier.string + "Members {\n");

    for (auto &member : statement->members)
    {
        source.append("struct " + member.identifier.string + " {\n");
        for (int i = 0; i < member.members.size(); i++)
        {
            source.append(emit_type_expression(member.members[i]) + " __" + std::to_string(i));
            source.append(";\n");
        }

        source.append("};\n");
    }
    source.append("}\n\n\n");

    constexpr auto overload_ops_template = R"(
bool operator==(const {}& other) const {{
        return this->index == other.index;
    }}

    bool operator!=(const {}& other) const {{
        return this->index != other.index;
    }}
)";

    /*
     * struct X {
     *      u64 index;
     *      union Members {
     *          __{EnumName}Members::Member __Member;
     *      } members;
     *
     *      {overload operators}
     * };
     */
    source.append("struct " + statement->identifier.string + " {\n");
    source.append("u64 index;\n");
    source.append("union Members {\n");
    for (auto &member : statement->members)
    {
        source.append(
            "__" + statement->identifier.string + "Members::" + member.identifier.string + " __" +
            member.identifier.string + ";\n"
        );
    }
    source.append("} members;\n");
    source.append(fmt::format(overload_ops_template, statement->identifier.string, statement->identifier.string));
    source.append("};\n");

    return source;
}

std::string CppBackend::emit_continue_statement(ContinueStatement *statement) {
    return "continue;";
}

std::string CppBackend::emit_alias_statement(AliasStatement *statement) {
    return "typedef " + emit_type_expression(statement->type_expression) + " " + statement->identifier.string + ";\n";
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
    case ExpressionType::EXPRESSION_GROUP:
        return emit_group_expression(dynamic_cast<GroupExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_NULL_LITERAL:
        return emit_null_literal_expression(dynamic_cast<NullLiteralExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_PROPAGATE:
        return emit_propagate_expression(dynamic_cast<PropagateExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_ZERO_LITERAL:
        return emit_zero_literal_expression(dynamic_cast<ZeroLiteralExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_FN:
        return emit_fn_expression(dynamic_cast<FnExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_INSTANTIATION:
        return emit_instantiate_expression(dynamic_cast<InstantiateExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_STRUCT_INSTANCE:
        return emit_struct_instance_expression(dynamic_cast<StructInstanceExpression *>(expression));
        break;
    case ExpressionType::EXPRESSION_ENUM_INSTANCE:
        return emit_enum_instance_expression(dynamic_cast<EnumInstanceExpression *>(expression));
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
    return "LiamInternal::make_str((char*)\"" + expression->token.string + "\", " +
           std::to_string(string_literal_length(&expression->token.string)) + ")";
}

std::string CppBackend::emit_bool_literal_expression(BoolLiteralExpression *expression) {
    return expression->value.string;
}

std::string CppBackend::emit_int_literal_expression(NumberLiteralExpression *expression) {
    auto number_type = static_cast<NumberTypeInfo *>(expression->type_info);

    std::string func_call = "LiamInternal::__";

    if (number_type->number_type == UNSIGNED)
    { func_call.append("u"); }
    else if (number_type->number_type == SIGNED)
    { func_call.append("i"); }
    else if (number_type->number_type == FLOAT)
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
    if (expression->op.type == TokenType::TOKEN_AT)
    { return "&(" + emit_expression(expression->expression) + ")"; }
    else if (expression->op.type == TokenType::TOKEN_STAR)
    { return "*(" + emit_expression(expression->expression) + ")"; }
    else if (expression->op.type == TokenType::TOKEN_NOT)
    { return "!(" + emit_expression(expression->expression) + ")"; }

    panic("Got a unrecognized operand in cpp backend");
    return "";
}

std::string CppBackend::emit_call_expression(CallExpression *expression) {
    auto source = std::string();
    source.append(emit_expression(expression->callee));

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

    if (expression->callee->type == ExpressionType::EXPRESSION_GET)
    {
        auto get_expression = dynamic_cast<GetExpression *>(expression->callee);

        // because we can call members of a type T on ^T sometimes we do not need to add
        // a & as it is already a pointer, if it is not a pointer then add the &
        if (get_expression->lhs->type_info->type != TypeInfoType::POINTER)
        { source.append("&"); }

        source.append(emit_expression(get_expression->lhs));
        if (expression->args.size() > 0)
        { source.append(", "); }
    }

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

    // to stop conflicts with member functions we emit __func for the get expression
    if (expression->type_info->type == TypeInfoType::FN)
    { return "__" + expression->member.string; }

    if (expression->lhs->type_info->type == TypeInfoType::POINTER) 
    { return emit_expression(expression->lhs) + "->" + expression->member.string; }

    if (expression->lhs->type_info->type == TypeInfoType::ENUM)
    { return emit_expression(expression->lhs) + "::" + expression->member.string; }

    return emit_expression(expression->lhs) + "." + expression->member.string;
}

std::string CppBackend::emit_group_expression(GroupExpression *expression) {
    return "(" + emit_expression(expression->expression) + ")";
}

std::string CppBackend::emit_null_literal_expression(NullLiteralExpression *expression) {
    return "nullptr";
}

std::string CppBackend::emit_propagate_expression(PropagateExpression *expression) {

    constexpr auto source_template = R"(
({{
        //                  vv expression given
        auto __evaluated = ({});
        //                            vv type expression given
        if (auto __temp = std::get_if<{}>(&__evaluated))
             return *__temp;

        //                                  vv type list from else type
        auto v = LiamInternal::cast_variant<{}>(__evaluated);
        {}
}})
)";

    constexpr auto single_type_source = R"(
    //          vv if type list is single then extract type
    std::get<{}>(v);
)";

    constexpr auto multi_type_source = R"(
    v;
)";

    std::string type_list;
    if (expression->otherwise->type == TypeExpressionType::TYPE_UNION)
    {
        auto union_expression = static_cast<UnionTypeExpression *>(expression->otherwise);

        // if the type expression is a union type expression then the type
        // needed is a comma seperated list of the types in the union
        for (u64 i = 0; i < union_expression->type_expressions.size(); i++)
        {
            type_list.append(emit_type_expression(union_expression->type_expressions[i]));
            if (i + 1 < union_expression->type_expressions.size())
            { type_list.append(", "); }
        }
    }
    else
    { type_list = emit_type_expression(expression->otherwise); }

    std::string source;
    if (expression->otherwise->type == TypeExpressionType::TYPE_UNION)
    {
        source = fmt::format(
            source_template, emit_expression(expression->expression), emit_type_expression(expression->type_expression),
            type_list, multi_type_source
        );
    }
    else
    {
        source = fmt::format(
            source_template, emit_expression(expression->expression), emit_type_expression(expression->type_expression),
            type_list, fmt::format(single_type_source, emit_type_expression(expression->otherwise))
        );
    }

    return source;
}

std::string CppBackend::emit_zero_literal_expression(ZeroLiteralExpression *expression) {
    return "{}";
}

std::string CppBackend::emit_fn_expression(FnExpression *expression) {
    std::string source = "[](";

    int index = 0;

    for (auto [identifier, type] : expression->params)
    {
        source.append(emit_type_expression(type) + " " + identifier.string);
        if (index + 1 < expression->params.size())
        { source.append(", "); }
        index++;
    }
    source.append(")");
    source.append(emit_scope_statement(expression->body));
    return source;
}

std::string CppBackend::emit_instantiate_expression(InstantiateExpression *expression) {
    auto source = std::string();

    // OwnedPtr(new {expr});
    if (expression->instantiate_type == InstantiateExpression::NEW)
    { source.append("LiamInternal::OwnedPtr(new "); }

    source.append(emit_expression(expression->expression));

    // add the final ) for tbe Owned Ptr constructor
    if (expression->instantiate_type == InstantiateExpression::NEW)
    { source.append(")"); }

    return source;
}

std::string CppBackend::emit_enum_instance_expression(EnumInstanceExpression *expression) {
    std::string source = "";

    /*
     * let expr := Expr::Number(100);
     * Expr expr = Expr{.index = 0, .members.__Number = __ExprMembers::Number{100}};
     */

    std::string enum_type = expression->lhs.string;

    source.append(
        enum_type + "{.index = " + std::to_string(expression->member_index) + ", .members.__" +
        expression->member.string + " = __" + enum_type + "Members::" + expression->member.string + "{"
    );

    int index = 0;

    for (auto expr : expression->arguments)
    {
        source.append(emit_expression(expr));
        if (index + 1 < expression->arguments.size())
        { source.append(", "); }
        index++;
    }

    source.append("}}");
    return source;
}

std::string CppBackend::emit_struct_instance_expression(StructInstanceExpression *expression) {
    std::string source;

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
    case TypeExpressionType::TYPE_FN:
        return emit_fn_type_expression(dynamic_cast<FnTypeExpression *>(type_expression));
        break;
    default:
        panic("Cpp back end does not support this type expression");
        return "";
    }
}

std::string CppBackend::emit_union_type_expression(UnionTypeExpression *type_expression) {
    std::string source = "std::variant";
    source.append(emit_cpp_template_params(&type_expression->type_expressions));
    return source;
}

std::string CppBackend::emit_unary_type_expression(UnaryTypeExpression *type_expression) {

    if (type_expression->unary_type == UnaryType::WEAK_POINTER)
    { return emit_type_expression(type_expression->type_expression) + "*"; }

    if (type_expression->unary_type == UnaryType::OWNED_POINTER)
    { return "LiamInternal::OwnedPtr<" + emit_type_expression(type_expression->type_expression) + ">"; }

    panic("Cpp backend does not support this op yet...");
    return "";
}

std::string CppBackend::emit_specified_generics_type_expression(SpecifiedGenericsTypeExpression *type_expression) {
    auto source = emit_type_expression(type_expression->struct_type);
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

std::string CppBackend::emit_fn_type_expression(FnTypeExpression *type_expression) {
    std::string source = "std::function<" + emit_type_expression(type_expression->return_type) + "(";

    int index = 0;
    for (auto type : type_expression->params)
    {
        source.append(emit_type_expression(type));
        if (index + 1 < type_expression->params.size())
        { source.append(", "); }
        index++;
    }
    source.append(")>");
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

u64 string_literal_length(std::string *string) {
    u64 length = 0;

    for (int i = 0; i < string->size(); i++)
    {
        if (string->at(i) == '\\' && i + 1 < string->size())
        { i += 2; }

        length++;
    }

    return length;
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
