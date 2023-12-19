#include "cpp_backend.h"

#include <algorithm>
#include <cstddef>
#include <format>
#include <string>
#include <vector>

#include "ast.h"
#include "baseLayer/debug.h"
#include "sorting_node.h"

CppBuilder::CppBuilder() {
    this->source      = std::string();
    this->indentation = 0;
}

void CppBuilder::start_line() {
    this->append_indentation();
}

void CppBuilder::end_line() {
    this->insert_new_line();
}

void CppBuilder::append(std::string string) {
    this->source.append(string);

#ifdef PRINT_CPP_BUILDER
    std::cout << string;
#endif
}

void CppBuilder::append_line(std::string string) {
    append_indentation();
    this->source.append(string);
    this->source.append("\n");

#ifdef PRINT_CPP_BUILDER
    std::cout << string << "\n";
#endif
}

void CppBuilder::insert_new_line() {
    this->source.append("\n");
#ifdef PRINT_CPP_BUILDER
    std::cout << "\n";
#endif
}

void CppBuilder::append_indentation() {
    for (u64 i = 0; i < this->indentation; i++) {
        this->source.append("    ");
#ifdef PRINT_CPP_BUILDER
        std::cout << "    ";
#endif
    }
}

void CppBuilder::indent() {
    this->indentation++;
}

void CppBuilder::un_indent() {
    ASSERT(this->indentation != 0);
    this->indentation--;
}

CppBackend::CppBackend() {
    this->compilation_unit = NULL;
    this->builder          = CppBuilder();
}

std::string CppBackend::emit(CompilationBundle *bundle) {
    this->compilation_bundle = bundle;

    this->builder.append_line("#include <core.h>");
    this->builder.append_line("#include <liam_stdlib.h>");

    for (CompilationUnit *cu : bundle->compilation_units) {
        this->compilation_unit = cu;
        forward_declare_namespace(cu);
    }

    for (CompilationUnit *cu : bundle->compilation_units) {
        this->compilation_unit = cu;
        // namespace imports
        for (auto stmt : this->compilation_unit->top_level_import_statements) {
            emit_import_statement(stmt);
        }
    }

    for (CompilationUnit *cu : bundle->compilation_units) {
        this->compilation_unit = cu;
        // forward declarations
        for (auto stmt : this->compilation_unit->top_level_struct_statements) {
            forward_declare_struct(stmt);
        }

        for (auto stmt : this->compilation_unit->top_level_fn_statements) {
            forward_declare_function(stmt);
        }
    }

    this->builder.insert_new_line();
    for (SortingNode &node : bundle->sorted_types) {
        this->compilation_unit = node.type_info->defined_location->compilation_unit;

        // struct bodies
        emit_struct_statement(node.type_info->defined_location);
    }

    this->builder.insert_new_line();
    for (CompilationUnit *cu : bundle->compilation_units) {
        this->compilation_unit = cu;

        // function bodies
        for (auto stmt : this->compilation_unit->top_level_fn_statements) {
            emit_fn_statement(stmt);
        }
    }
    this->builder.append_line("\nint main(int argc, char **argv) { start::main(); return 0; }");
    return this->builder.source;
}

void CppBackend::forward_declare_namespace(CompilationUnit *compilation_unit) {
    // namespace main {}
    this->builder.start_line();
    this->builder.append(std::format("namespace {} {{ }}", get_namespace_name(this->compilation_unit)));
    this->builder.end_line();
}

void CppBackend::forward_declare_struct(StructStatement *statement) {
    this->builder.start_line();
    this->builder.append(std::format("namespace {} {{ ", get_namespace_name(this->compilation_unit)));
    this->builder.append("struct " + this->compilation_unit->get_token_string_from_index(statement->identifier) +
                         "; }");
    this->builder.end_line();
}

void CppBackend::forward_declare_function(FnStatement *statement) {
    // namespace main { i64 main(i64 a); }
    this->builder.start_line();

    // namespace main {
    this->builder.append(std::format("namespace {} {{ ", get_namespace_name(this->compilation_unit)));

    // i64
    emit_type_expression(statement->return_type);
    this->builder.append(" ");

    std::string name = this->compilation_unit->get_token_string_from_index(statement->identifier);

    // main(
    this->builder.append(name + "(");

    u64 index = 0;
    for (auto [token_index, type] : statement->params) {
        std::string identifier_string = this->compilation_unit->get_token_string_from_index(token_index);

        // i64 a,
        emit_type_expression(type);
        this->builder.append(" ");
        this->builder.append(identifier_string);
        if (index + 1 < statement->params.size()) {
            this->builder.append(", ");
        }
        index++;
    }

    // ); }
    this->builder.append("); }");
    this->builder.end_line();
}

void CppBackend::emit_statement(Statement *statement) {
    switch (statement->statement_type) {
    case StatementType::RETURN:
        emit_return_statement(static_cast<ReturnStatement *>(statement));
        break;
    case StatementType::BREAK:
        emit_break_statement(static_cast<BreakStatement *>(statement));
        break;
    case StatementType::LET:
        emit_let_statement(static_cast<LetStatement *>(statement));
        break;
    case StatementType::FN:
        emit_fn_statement(static_cast<FnStatement *>(statement));
        break;
    case StatementType::SCOPE:
        emit_scope_statement(static_cast<ScopeStatement *>(statement));
        break;
    case StatementType::STRUCT:
        emit_struct_statement(static_cast<StructStatement *>(statement));
        break;
    case StatementType::ASSIGNMENT:
        emit_assigment_statement(static_cast<AssigmentStatement *>(statement));
        break;
    case StatementType::EXPRESSION:
        emit_expression_statement(static_cast<ExpressionStatement *>(statement));
        break;
    case StatementType::FOR:
        emit_for_statement(static_cast<ForStatement *>(statement));
        break;
    case StatementType::IF:
        emit_if_statement(static_cast<IfStatement *>(statement));
        break;
    case StatementType::CONTINUE:
        emit_continue_statement(static_cast<ContinueStatement *>(statement));
        break;
    case StatementType::PRINT:
        emit_print_statement(static_cast<PrintStatement *>(statement));
        break;
    default:
        UNREACHABLE();
    }
}

void CppBackend::emit_import_statement(ImportStatement *statement) {
    // namespace main { namespace new_name = other; }
    this->builder.start_line();

    // namespace main { namespace
    this->builder.append(std::format("namespace {} {{ namespace ", get_namespace_name(this->compilation_unit)));

    // new_name =
    std::string new_name = this->compilation_unit->get_token_string_from_index(statement->identifier);
    this->builder.append(new_name);
    this->builder.append(" = ");

    std::string other_namespace_name = get_namespace_name(
        this->compilation_bundle->compilation_units[statement->namespace_type_info->compilation_unit_index]);

    // other
    this->builder.append(other_namespace_name);

    // ; }
    this->builder.append("; }");
    this->builder.end_line();
}

void CppBackend::emit_return_statement(ReturnStatement *statement) {
    this->builder.start_line();
    this->builder.append("return ");
    if (statement->expression) {
        emit_expression(statement->expression);
    }
    this->builder.append(";");
    this->builder.end_line();
}

void CppBackend::emit_break_statement(BreakStatement *statement) {
    this->builder.append_line("break;");
}

void CppBackend::emit_let_statement(LetStatement *statement) {
    this->builder.start_line();
    if (statement->type) {
        emit_type_expression(statement->type);
    } else {
        builder.append("auto");
    }

    builder.append(" ");
    builder.append(this->compilation_unit->get_token_string_from_index(statement->identifier));
    builder.append(" = ");
    emit_expression(statement->rhs);
    builder.append(";");
    builder.end_line();
}

void CppBackend::emit_scope_statement(ScopeStatement *statement) {
    this->builder.append_line("{");
    this->builder.indent();
    for (auto stmt : statement->statements) {
        emit_statement(stmt);
    }
    this->builder.un_indent();
    this->builder.append_line("}");
}

void CppBackend::emit_fn_statement(FnStatement *statement) {
    // namespace main {
    //      void main(i64 a) {
    //          ...
    //      }
    // }

    // namespace main {
    this->builder.append_line(std::format("namespace {} {{", get_namespace_name(this->compilation_unit)));

    //      void main(i64 a) {
    this->builder.indent();
    this->builder.start_line();
    emit_type_expression(statement->return_type);
    this->builder.append(" ");
    this->builder.append(this->compilation_unit->get_token_string_from_index(statement->identifier));
    this->builder.append("(");

    // params of the function
    u64 index = 0;
    for (auto [identifier, type] : statement->params) {
        std::string identifier_string = this->compilation_unit->get_token_string_from_index(identifier);
        emit_type_expression(type);
        this->builder.append(" ");
        this->builder.append(identifier_string);
        if (index + 1 < statement->params.size()) {
            this->builder.append(", ");
        }
        index++;
    }

    this->builder.append(")");
    this->builder.end_line();

    // function body:
    //          ...
    this->emit_scope_statement(statement->body);

    this->builder.un_indent();

    // }
    this->builder.append_line("}");
}

void CppBackend::emit_struct_statement(StructStatement *statement) {
    // namespace main {
    //      struct Main {
    //          a: i64,
    //          b: i64
    //      };
    // }

    // namespace main {
    this->builder.append_line(std::format("namespace {} {{", get_namespace_name(this->compilation_unit)));

    //      struct Main {
    this->builder.indent();
    this->builder.append_line("struct " + this->compilation_unit->get_token_string_from_index(statement->identifier) +
                              " {");

    //          a: i64,
    //          b: i64
    this->builder.indent();
    for (auto [identifier_token_index, type] : statement->members) {
        this->builder.start_line();
        emit_type_expression(type);
        this->builder.append(" ");
        this->builder.append(this->compilation_unit->get_token_string_from_index(identifier_token_index) + ";");
        this->builder.end_line();
    }
    this->builder.un_indent();

    // closing of the struct body
    this->builder.append_line("};");
    this->builder.un_indent();

    this->builder.append_line("}");
}

void CppBackend::emit_assigment_statement(AssigmentStatement *statement) {
    this->builder.start_line();
    emit_expression(statement->lhs);
    this->builder.append(" = ");
    emit_expression(statement->assigned_to->expression);
    this->builder.append(";");
    this->builder.end_line();
}

void CppBackend::emit_expression_statement(ExpressionStatement *statement) {
    this->builder.start_line();
    emit_expression(statement->expression);
    this->builder.append(";");
    this->builder.end_line();
}

void CppBackend::emit_for_statement(ForStatement *statement) {

    // for value : array { ... }
    // generates:
    // {
    //      auto __value_a = &array;
    //      for (u64 __value_i = 0; i < __value_a->size; __value_i++) {
    //          auto value = (*__value_a)[__value_i];
    //          { ... }
    //      }
    // }

    // compiler generated names used for the for loop
    // the value_name is only used when the expression passed is a r value and
    // we want to store it first

    // __value_i
    std::string indexer =
        std::format("__{}_i", this->compilation_unit->get_token_string_from_index(statement->value_identifier));

    // __value_a
    std::string to_be_indexed =
        std::format("__{}_a", this->compilation_unit->get_token_string_from_index(statement->value_identifier));

    // value
    std::string value_identifier = this->compilation_unit->get_token_string_from_index(statement->value_identifier);

    bool iterating_over_r_value  = statement->expression->category == ExpressionCategory::RVALUE;

    // the outer scope wrapping the for loop so we can get the to be indexed varaible
    //      auto __value_a = &array;
    // if it is an r value do not use & as we are storing it in __value_a not getting the pointer
    this->builder.append_line("{");
    this->builder.indent();
    this->builder.start_line();
    if (iterating_over_r_value) {
        this->builder.append(std::format("auto {} = ", to_be_indexed));
    } else {
        this->builder.append(std::format("auto {} = &", to_be_indexed));
    }
    emit_expression(statement->expression);
    this->builder.append(";");
    this->builder.end_line();

    this->builder.start_line();
    this->builder.append("for (");
    this->builder.end_line();

    this->builder.indent();

    //     u64 __value_i = 0;
    this->builder.start_line();
    this->builder.append(std::format("u64 {} = 0;", indexer));
    this->builder.end_line();

    //    __value_i < __value_a->size;
    // if __value_a is a pointer then use -> else .
    this->builder.start_line();
    this->builder.append(std::format("{} < ", indexer));
    if (iterating_over_r_value) {
        this->builder.append(std::format("{}.size;", to_be_indexed));
    } else {
        this->builder.append(std::format("{}->size;", to_be_indexed));
    }
    this->builder.end_line();

    //    __value_i++
    this->builder.start_line();
    this->builder.append(std::format("{}++", indexer));
    this->builder.end_line();

    this->builder.un_indent();
    this->builder.append_line(")");

    //      auto value = (*__value_a)[__value_i];
    // if it is a r value then do not dereference as a pointer
    this->builder.start_line();
    this->builder.append(std::format("{{ auto {} = ", value_identifier));
    if (iterating_over_r_value) {
        this->builder.append(std::format("{}[{}];", to_be_indexed, indexer));
    } else {
        this->builder.append(std::format("(*{})[{}];", to_be_indexed, indexer));
    }
    this->builder.end_line();

    emit_scope_statement(statement->body);

    // closes the outside scope where we assign value
    this->builder.append_line("}");

    // closes the outside scope that we wrap the for in to create the __value_a variable
    this->builder.un_indent();
    this->builder.append_line("}");
}

void CppBackend::emit_if_statement(IfStatement *statement) {
    builder.start_line();
    builder.append("if (");
    emit_expression(statement->expression);
    builder.append(")");
    builder.end_line();

    emit_scope_statement(statement->body);

    if (statement->else_statement) {
        emit_else_statement(statement->else_statement);
    }
}

void CppBackend::emit_else_statement(ElseStatement *statement) {
    builder.start_line();
    builder.append("else");
    builder.end_line();
    if (statement->if_statement) {
        emit_if_statement(statement->if_statement);
    } else if (statement->body) {
        emit_scope_statement(statement->body);
    }
}

void CppBackend::emit_continue_statement(ContinueStatement *statement) {
    this->builder.append_line("continue;");
}

void CppBackend::emit_print_statement(PrintStatement *statement) {
    this->builder.start_line();
    this->builder.append("std::cout << ");
    emit_expression(statement->expression);
    this->builder.append(" << std::endl;");
    this->builder.end_line();
}

void CppBackend::emit_expression(Expression *expression) {
    switch (expression->type) {
    case ExpressionType::STRING_LITERAL:
        emit_string_literal_expression(static_cast<StringLiteralExpression *>(expression));
        break;
    case ExpressionType::NUMBER_LITERAL:
        emit_int_literal_expression(static_cast<NumberLiteralExpression *>(expression));
        break;
    case ExpressionType::BOOL_LITERAL:
        emit_bool_literal_expression(static_cast<BoolLiteralExpression *>(expression));
        break;
    case ExpressionType::CALL:
        emit_call_expression(static_cast<CallExpression *>(expression));
        break;
    case ExpressionType::IDENTIFIER:
        emit_identifier_expression(static_cast<IdentifierExpression *>(expression));
        break;
    case ExpressionType::BINARY:
        emit_binary_expression(static_cast<BinaryExpression *>(expression));
        break;
    case ExpressionType::UNARY:
        emit_unary_expression(static_cast<UnaryExpression *>(expression));
        break;
    case ExpressionType::GET:
        emit_get_expression(static_cast<GetExpression *>(expression));
        break;
    case ExpressionType::GROUP:
        emit_group_expression(static_cast<GroupExpression *>(expression));
        break;
    case ExpressionType::NULL_LITERAL:
        emit_null_literal_expression(static_cast<NullLiteralExpression *>(expression));
        break;
    case ExpressionType::ZERO_LITERAL:
        emit_zero_literal_expression(static_cast<ZeroLiteralExpression *>(expression));
        break;
    case ExpressionType::INSTANTIATION:
        emit_instantiate_expression(static_cast<InstantiateExpression *>(expression));
        break;
    case ExpressionType::STRUCT_INSTANCE:
        emit_struct_instance_expression(static_cast<StructInstanceExpression *>(expression));
        break;
    case ExpressionType::STATIC_ARRAY:
        emit_static_array_literal_expression(static_cast<StaticArrayExpression *>(expression));
        break;
    case ExpressionType::SUBSCRIPT:
        emit_subscript_expression(static_cast<SubscriptExpression *>(expression));
        break;
    default: {
        UNREACHABLE();
    }
    }
}

void CppBackend::emit_binary_expression(BinaryExpression *expression) {
    std::string op;
    switch (expression->op) {
    case TokenType::TOKEN_PLUS:
        op = "+";
        break;
    case TokenType::TOKEN_MINUS:
        op = "-";
        break;
    case TokenType::TOKEN_STAR:
        op = "-";
        break;
    case TokenType::TOKEN_SLASH:
        op = "/";
        break;
    case TokenType::TOKEN_MOD:
        op = "%";
        break;
    case TokenType::TOKEN_OR:
        op = "||";
        break;
    case TokenType::TOKEN_AND:
        op = "&&";
        break;
    case TokenType::TOKEN_EQUAL:
        op = "==";
        break;
    case TokenType::TOKEN_NOT_EQUAL:
        op = "!=";
        break;
    case TokenType::TOKEN_LESS:
        op = "<";
        break;
    case TokenType::TOKEN_GREATER:
        op = ">";
        break;
    case TokenType::TOKEN_GREATER_EQUAL:
        op = ">=";
        break;
    case TokenType::TOKEN_LESS_EQUAL:
        op = "<=";
        break;
    default:
        UNREACHABLE();
        break;
    }

    emit_expression(expression->left);
    this->builder.append(std::format(" {} ", op));
    emit_expression(expression->right);
}

void CppBackend::emit_string_literal_expression(StringLiteralExpression *expression) {
    std::string literal_string = this->compilation_unit->get_token_string_from_index(expression->token);

    this->builder.append("Liam::Slice((u8*)");
    this->builder.append(literal_string);
    this->builder.append(std::format(", {})", string_literal_length(&literal_string)));
}

void CppBackend::emit_bool_literal_expression(BoolLiteralExpression *expression) {
    this->builder.append(this->compilation_unit->get_token_string_from_index(expression->token));
}

void CppBackend::emit_int_literal_expression(NumberLiteralExpression *expression) {
    auto number_type = static_cast<NumberTypeInfo *>(expression->type_info);

    this->builder.append("Liam::make_");

    if (number_type->number_type == NumberType::UNSIGNED) {
        this->builder.append("u");
    } else if (number_type->number_type == NumberType::SIGNED) {
        this->builder.append("i");
    } else if (number_type->number_type == NumberType::FLOAT) {
        this->builder.append("f");
    }

    if (number_type->size == 8) {
        this->builder.append("8");
    } else if (number_type->size == 16) {
        this->builder.append("16");
    } else if (number_type->size == 32) {
        this->builder.append("32");
    } else if (number_type->size == 64) {
        this->builder.append("64");
    }

    this->builder.append("(" + std::to_string(expression->number) + ")");
}

void CppBackend::emit_unary_expression(UnaryExpression *expression) {
    if (expression->op == TokenType::TOKEN_AMPERSAND) {
        this->builder.append("&(");
        emit_expression(expression->expression);
        this->builder.append(")");
    } else if (expression->op == TokenType::TOKEN_STAR) {
        this->builder.append("*(");
        emit_expression(expression->expression);
        this->builder.append(")");
    } else if (expression->op == TokenType::TOKEN_NOT) {
        this->builder.append("!(");
        emit_expression(expression->expression);
        this->builder.append(")");
    } else {
        UNREACHABLE();
    }
}

void CppBackend::emit_call_expression(CallExpression *expression) {
    emit_expression(expression->callee);
    this->builder.append("(");

    u64 index = 0;
    for (auto expr : expression->args) {
        emit_expression(expr);

        if (index + 1 < expression->args.size()) {
            this->builder.append(", ");
        }
        index++;
    }
    this->builder.append(")");
}

void CppBackend::emit_identifier_expression(IdentifierExpression *expression) {
    this->builder.append(this->compilation_unit->get_token_string_from_index(expression->identifier));
}

void CppBackend::emit_get_expression(GetExpression *expression) {
    std::string member_string = this->compilation_unit->get_token_string_from_index(expression->member);

    emit_expression(expression->lhs);

    if (expression->lhs->type_info->type == TypeInfoType::POINTER) {
        this->builder.append("->");
    } else if (expression->lhs->type_info->type == TypeInfoType::NAMESPACE) {
        this->builder.append("::");
    } else {
        this->builder.append(".");
    }

    this->builder.append(member_string);
}

void CppBackend::emit_group_expression(GroupExpression *expression) {
    this->builder.append("(");
    emit_expression(expression->sub_expression);
    this->builder.append(")");
}

void CppBackend::emit_null_literal_expression(NullLiteralExpression *expression) {
    this->builder.append("nullptr");
}

void CppBackend::emit_zero_literal_expression(ZeroLiteralExpression *expression) {
    this->builder.append("{}");
}

void CppBackend::emit_instantiate_expression(InstantiateExpression *expression) {
    emit_expression(expression->expression);
}

void CppBackend::emit_struct_instance_expression(StructInstanceExpression *expression) {
    emit_type_expression(expression->type_expression);
    this->builder.append("{");
    u64 index = 0;
    for (auto [name, expr] : expression->named_expressions) {
        emit_expression(expr);
        if (index + 1 < expression->named_expressions.size()) {
            this->builder.append(", ");
        }
        index++;
    }
    this->builder.append("}");
}

void CppBackend::emit_static_array_literal_expression(StaticArrayExpression *expression) {
    this->builder.append("Liam::StaticArray<");
    emit_int_literal_expression(expression->number);
    this->builder.append(", ");
    emit_type_expression(expression->type_expression);
    this->builder.append(">(std::initializer_list<");
    emit_type_expression(expression->type_expression);
    this->builder.append(">({");

    // expression list
    u64 index = 0;
    for (auto expr : expression->expressions) {
        emit_expression(expr);

        if (index + 1 < expression->expressions.size()) {
            this->builder.append(", ");
        }
        index++;
    }

    this->builder.append("}))");
}

void CppBackend::emit_subscript_expression(SubscriptExpression *expression) {
    emit_expression(expression->subscriptee);
    if (expression->subscripter->type != ExpressionType::RANGE) {
        this->builder.append("[");
        emit_expression(expression->subscripter);
        this->builder.append("]");
        return;
    } else {
        ASSERT(expression->subscripter->type == ExpressionType::RANGE);
        emit_range_slicing_expression(static_cast<RangeExpression *>(expression->subscripter));
    }
}

void CppBackend::emit_range_slicing_expression(RangeExpression *expression) {
    // there are 4 cases we need to handle
    // 1. [a..b] -> slice_with_start_and_end(a, b)
    // 2. [a..] -> slice_with_start(a)
    // 3. [..b] -> slice_with_end(b)
    // 4. [..] -> slice_full()
    if (expression->start && expression->end) {
        this->builder.append(".slice_with_start_and_end(");
        emit_expression(expression->start);
        this->builder.append(", ");
        emit_expression(expression->end);
        this->builder.append(")");
    } else if (expression->start) {
        this->builder.append(".slice_with_start(");
        emit_expression(expression->start);
        this->builder.append(")");
    } else if (expression->end) {
        this->builder.append(".slice_with_end(");
        emit_expression(expression->end);
        this->builder.append(")");
    } else {
        this->builder.append(".slice_full()");
    }
}

void CppBackend::emit_type_expression(TypeExpression *type_expression) {
    switch (type_expression->type) {
    case TypeExpressionType::TYPE_IDENTIFIER:
        emit_identifier_type_expression(static_cast<IdentifierTypeExpression *>(type_expression));
        break;
    case TypeExpressionType::TYPE_UNARY:
        emit_unary_type_expression(static_cast<UnaryTypeExpression *>(type_expression));
        break;
    case TypeExpressionType::TYPE_GET:
        emit_get_type_expression(static_cast<GetTypeExpression *>(type_expression));
        break;
    case TypeExpressionType::TYPE_STATIC_ARRAY:
        emit_static_array_type_expression(static_cast<StaticArrayTypeExpression *>(type_expression));
        break;
    case TypeExpressionType::TYPE_SLICE:
        emit_slice_type_expression(static_cast<SliceTypeExpression *>(type_expression));
        break;
    default:
        UNREACHABLE();
    }
}

void CppBackend::emit_unary_type_expression(UnaryTypeExpression *type_expression) {
    if (type_expression->unary_type == UnaryType::POINTER) {
        emit_type_expression(type_expression->type_expression);
        this->builder.append("*");
    } else {
        UNREACHABLE();
    }
}

void CppBackend::emit_identifier_type_expression(IdentifierTypeExpression *type_expression) {
    this->builder.append(this->compilation_unit->get_token_string_from_index(type_expression->identifier));
}

void CppBackend::emit_get_type_expression(GetTypeExpression *type_expression) {
    std::string identifier = this->compilation_unit->get_token_string_from_index(type_expression->identifier);

    emit_type_expression(type_expression->type_expression);

    if (type_expression->type_expression->type_info->type == TypeInfoType::NAMESPACE) {
        this->builder.append("::");
    } else {
        UNREACHABLE();
    }

    this->builder.append(identifier);
}

void CppBackend::emit_static_array_type_expression(StaticArrayTypeExpression *type_expression) {
    this->builder.append("Liam::StaticArray<");
    emit_int_literal_expression(type_expression->size);
    this->builder.append(", ");
    emit_type_expression(type_expression->base_type);
    this->builder.append(">");
}

void CppBackend::emit_slice_type_expression(SliceTypeExpression *type_expression) {
    this->builder.append("Liam::Slice<");
    emit_type_expression(type_expression->base_type);
    this->builder.append(">");
}

std::string strip_semi_colon(std::string str) {
    if (str.size() == 0)
        return str;

    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());

    if (str.back() == ';') {
        return str.substr(0, str.size() - 1);
    } else {
        return str;
    }
    // return (str[str.size() - 1] == ';') ? str.substr(0, str.size()-1) : str;
}

u64 string_literal_length(std::string *string) {
    // FIXME: this will not be correct for strings that have escaped characters
    // in them for example \" should only count as 1 character.
    //
    // We remove the 2 because we need to remove the quotes from the string
    return string->size() - 2;
}

std::string get_namespace_name(CompilationUnit *compilation_unit) {
    return compilation_unit->file_data->absolute_path.stem();
}
