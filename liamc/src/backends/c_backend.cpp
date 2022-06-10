#include "backends/c_backend.h"

std::string CBackend::
emit(File* file) {
    auto source_generated = std::string("#include <stdio.h>\n"
                                        "typedef unsigned long u64;\n"
                                        "#define true 1\n"
                                        "#define false 0\n\n\n");

    for (auto stmt : file->statements) {
        source_generated.append(emit_statement(stmt));
    }

    return source_generated;
}

std::string CBackend::
emit_statement(Statement* statement) {
    switch (statement->statement_type) {
        case StatementType::STATEMENT_INSERT:
            return emit_insert_statement(dynamic_cast<InsertStatement*>(statement));
            break;
        case StatementType::STATEMENT_RETURN:
            return emit_return_statement(dynamic_cast<ReturnStatement*>(statement));
            break;
        case StatementType::STATEMENT_BREAK:
            return emit_break_statement(dynamic_cast<BreakStatement*>(statement));
            break;
        case StatementType::STATEMENT_LET:
            return emit_let_statement(dynamic_cast<LetStatement*>(statement));
            break;
        case StatementType::STATEMENT_FN:
            return emit_fn_statement(dynamic_cast<FnStatement*>(statement));
            break;
        case StatementType::STATEMENT_SCOPE:
            return emit_scope_statement(dynamic_cast<ScopeStatement*>(statement));
            break;
        case StatementType::STATEMENT_LOOP:
            return emit_loop_statement(dynamic_cast<LoopStatement*>(statement));
            break;
        case StatementType::STATEMENT_STRUCT:
            return emit_struct_statement(dynamic_cast<StructStatement*>(statement));
            break;
        case StatementType::STATEMENT_ASSIGNMENT:
            return emit_assigment_statement(dynamic_cast<AssigmentStatement*>(statement));
            break;
        case StatementType::STATEMENT_EXPRESSION:
            return emit_expression_statement(dynamic_cast<ExpressionStatement*>(statement));
            break;
        case StatementType::STATEMENT_FOR:
            return emit_for_statement(dynamic_cast<ForStatement*>(statement));
            break;
        case StatementType::STATEMENT_IF:
            return emit_if_statement(dynamic_cast<IfStatement*>(statement));
            break;
    }


    panic("Statement not implemented in c back end :[");
    return "";
}

std::string CBackend::
emit_insert_statement(InsertStatement* statement) {
    if(statement->byte_code->type != ExpressionType::EXPRESSION_STRING_LITERAL) {
        panic("Cannot emit non string literal in insert");
    }

    auto string_literal = dynamic_cast<StringLiteralExpression*>(statement->byte_code);
    return string_literal->token.string;
}

std::string CBackend::
emit_return_statement(ReturnStatement* statement) {
    return "return " + emit_expression(statement->expression) + ";";
}

std::string CBackend::
emit_break_statement(BreakStatement* statement) {
    panic("Not implemented in C backend");
    return "";
}

std::string CBackend::
emit_let_statement(LetStatement* statement) {
    auto source = std::string();
    source.append(emit_type_expression(statement->type) + " ");
    source.append(statement->identifier.string);
    source.append(" = ");
    source.append(emit_expression(statement->rhs) + ";\n");
    return source;
}

std::string CBackend::
emit_scope_statement(ScopeStatement* statement) {
    auto source = std::string();
    source.append("{\n");
    for(auto stmt : statement->statements) {
        source.append(emit_statement(stmt));
    }
    source.append("\n}\n\n");
    return source;
}

std::string CBackend::
emit_fn_statement(FnStatement* statement) {
    auto source = std::string();
    source.append(emit_type_expression(statement->return_type) + " ");
    source.append(statement->identifier.string);
    source.append("(");
    int index = 0;
    for(auto [identifier, type] : statement->params) {
        source.append(emit_type_expression(type) + " " + identifier.string);
        if(index + 1 < statement->params.size()) {
            source.append(", ");
        }
        index++;
    }
    source.append(")");
    source.append(emit_scope_statement(statement->body));
    return source;
}

std::string CBackend::
emit_loop_statement(LoopStatement* statement) {
    panic("Not implemented in C backend");
    return "";
}

std::string CBackend::
emit_struct_statement(StructStatement* statement) {
    auto source = std::string();
    source.append("typedef struct " + statement->identifier.string + " {");
    for(auto [identifier, type] : statement->members) {
        source.append("\n" + emit_type_expression(type) + " ");
        source.append(identifier.string + ";");
    }
    source.append("\n} " + statement->identifier.string + ";\n\n");
    return source;
}

std::string CBackend::
emit_assigment_statement(AssigmentStatement* statement) {
    panic("Not implemented in C backend");
    return "";
}

std::string CBackend::
emit_expression_statement(ExpressionStatement* statement) {
    return emit_expression(statement->expression) + ";\n";
}

std::string CBackend::
emit_for_statement(ForStatement* statement) {
    panic("Not implemented in C backend");
    return "";
}

std::string CBackend::
emit_if_statement(IfStatement* statement) {
    panic("Not implemented in C backend");
    return "";
}


std::string CBackend::
emit_expression(Expression* expression) {
    switch (expression->type) {
        case ExpressionType::EXPRESSION_STRING_LITERAL:
            return emit_string_literal_expression(dynamic_cast<StringLiteralExpression*>(expression));
            break;
        case ExpressionType::EXPRESSION_INT_LITERAL:
            return emit_int_literal_expression(dynamic_cast<IntLiteralExpression*>(expression));
            break;
        case ExpressionType::EXPRESSION_BOOL_LITERAL:
            return emit_bool_literal_expression(dynamic_cast<BoolLiteralExpression*>(expression));
            break;
        case ExpressionType::EXPRESSION_CALL:
            return emit_call_expression(dynamic_cast<CallExpression*>(expression));
            break;
        case ExpressionType::EXPRESSION_IDENTIFIER:
            return emit_identifier_expression(dynamic_cast<IdentifierExpression*>(expression));
            break;
        case ExpressionType::EXPRESSION_BINARY:
            return emit_binary_expression(dynamic_cast<BinaryExpression*>(expression));
            break;
        case ExpressionType::EXPRESSION_UNARY:
            return emit_unary_expression(dynamic_cast<UnaryExpression*>(expression));
            break;
        case ExpressionType::EXPRESSION_GET:
            return emit_get_expression(dynamic_cast<GetExpression*>(expression));
            break;
        case ExpressionType::EXPRESSION_NEW:
            return emit_new_expression(dynamic_cast<NewExpression*>(expression));
            break;
        case ExpressionType::EXPRESSION_ARRAY:
            return emit_array_expression(dynamic_cast<ArrayExpression*>(expression));
            break;
        case ExpressionType::EXPRESSION_ARRAY_SUBSCRIPT:
            return emit_array_subscript_expression(dynamic_cast<ArraySubscriptExpression*>(expression));
            break;
        case ExpressionType::EXPRESSION_GROUP:
            return emit_group_expression(dynamic_cast<GroupExpression*>(expression));
            break;
        default:
            return "";
    }
}

std::string CBackend::
emit_cloneable_expression(Expression* expression) {
    panic("Not implemented in C backend");
    return "";
}

std::string CBackend::
emit_binary_expression(BinaryExpression* expression) {
    panic("Not implemented in C backend");
    return "";
}

std::string CBackend::
emit_string_literal_expression(StringLiteralExpression* expression) {
    panic("Not implemented in C backend");
    return "";
}

std::string CBackend::
emit_bool_literal_expression(BoolLiteralExpression* expression) {
    panic("Not implemented in C backend");
    return "";
}

std::string CBackend::
emit_int_literal_expression(IntLiteralExpression* expression) {
    return expression->token.string;
}

std::string CBackend::
emit_unary_expression(UnaryExpression* expression) {
    if (expression->op.type == TOKEN_AT) {
        return "&(" + emit_expression(expression->expression) + ")";
    }
    else if (expression->op.type == TOKEN_STAR) {
        return "*(" + emit_expression(expression->expression) + ")";
    }

    panic("Got a unrecognized operand");
    return "";
}

std::string CBackend::
emit_call_expression(CallExpression* expression) {
    auto source = std::string();
    source.append(emit_expression(expression->identifier) + "(");
    int index = 0;
    for(auto expr : expression->args) {

        source.append(emit_expression(expr));

        if(index + 1 < expression->args.size()) {
            source.append(", ");
        }
        index++;
    }
    source.append(")");
    return source;
}

std::string CBackend::
emit_identifier_expression(IdentifierExpression* expression) {
    return expression->identifier.string;
}

std::string CBackend::
emit_get_expression(GetExpression* expression) {
    panic("Not implemented in C backend");
    return "";
}

std::string CBackend::
emit_new_expression(NewExpression* expression) {
    auto source = std::string();
    source.append("(" + expression->identifier.string + ") ");
    source.append("{");
    int index = 0;
    for(auto expr : expression->expressions) {

        source.append(emit_expression(expr));

        if(index + 1 < expression->expressions.size()) {
            source.append(", ");
        }
        index++;
    }
    source.append("}");
    return source;
}

std::string CBackend::
emit_array_expression(ArrayExpression* expression) {
    panic("Not implemented in C backend");
    return "";
}

std::string CBackend::
emit_array_subscript_expression(ArraySubscriptExpression* expression) {
    panic("Not implemented in C backend");
    return "";
}

std::string CBackend::
emit_group_expression(GroupExpression* expression) {
    panic("Not implemented in C backend");
    return "";
}


std::string CBackend::
emit_type_expression(TypeExpression* type_expression) {
    switch (type_expression->type) {
        case TypeExpressionType::TYPE_IDENTIFIER:
            return emit_identifier_type_expression(dynamic_cast<IdentifierTypeExpression*>(type_expression)); break;
        case TypeExpressionType::TYPE_POINTER:
            return emit_pointer_type_expression(dynamic_cast<PointerTypeExpression*>(type_expression)); break;
        case TypeExpressionType::TYPE_ARRAY:
            return emit_array_type_expression(dynamic_cast<ArrayTypeExpression*>(type_expression)); break;
        default:
            panic("C back end does not support this return_type expression");
    }
}

std::string CBackend::
emit_identifier_type_expression(IdentifierTypeExpression* type_expression) {
    return type_expression->identifier.string;
}

std::string CBackend::
emit_pointer_type_expression(PointerTypeExpression* type_expression) {
    return emit_type_expression(type_expression->pointer_of) + "*";
}

std::string CBackend::
emit_array_type_expression(ArrayTypeExpression* type_expression) {
    panic("Not implemented in C backend");
    return "";
}
