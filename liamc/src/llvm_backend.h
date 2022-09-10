#pragma once

#include "expression.h"
#include "parser.h"
#include "statement.h"
#include "type_checker.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

struct LLVMBackend {

    llvm::LLVMContext *context;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::Module> module;

    std::map<std::string, llvm::Function *> function_map;
    std::map<std::string, llvm::StructType *> struct_map;

    LLVMBackend();

    void emit(File *file);

    void emit_statement(Statement *statement);
    std::string emit_insert_statement(InsertStatement *statement);
    void emit_return_statement(ReturnStatement *statement);
    std::string emit_break_statement(BreakStatement *statement);
    void emit_let_statement(LetStatement *statement);
    void emit_scope_statement(ScopeStatement *statement);
    void emit_fn_statement(FnStatement *statement);
    void emit_struct_statement(StructStatement *statement);
    std::string emit_assigment_statement(AssigmentStatement *statement);
    void emit_expression_statement(ExpressionStatement *statement);
    std::string emit_for_statement(ForStatement *statement);
    void emit_if_statement(IfStatement *statement);
    std::string emit_else_statement(ElseStatement *statement);
    std::string emit_enum_statement(EnumStatement *statement);
    std::string emit_continue_statement(ContinueStatement *statement);
    std::string emit_alias_statement(AliasStatement *statement);
    std::string emit_test_statement(TestStatement *statement);

    llvm::Value *emit_expression(Expression *expression);
    llvm::Value *emit_is_expression(IsExpression *expression);
    llvm::Value *emit_binary_expression(BinaryExpression *expression);
    llvm::Value *emit_string_literal_expression(StringLiteralExpression *expression);
    llvm::Value *emit_bool_literal_expression(BoolLiteralExpression *expression);
    llvm::Value *emit_int_literal_expression(NumberLiteralExpression *expression);
    llvm::Value *emit_unary_expression(UnaryExpression *expression);
    llvm::Value *emit_call_expression(CallExpression *expression);
    llvm::Value *emit_identifier_expression(IdentifierExpression *expression);
    llvm::Value *emit_get_expression(GetExpression *expression);
    llvm::Value *emit_new_expression(NewExpression *expression);
    llvm::Value *emit_group_expression(GroupExpression *expression);
    llvm::Value *emit_null_literal_expression(NullLiteralExpression *expression);
    llvm::Value *emit_propagate_expression(PropagateExpression *expression);
    llvm::Value *emit_zero_literal_expression(ZeroLiteralExpression *expression);

    llvm::Type *emit_type_expression(TypeExpression *type_expression);
    llvm::Type *emit_unary_type_expression(UnaryTypeExpression *type_expression);
    llvm::Type *emit_union_type_expression(UnionTypeExpression *type_expression);
    llvm::Type *emit_specified_generics_type_expression(SpecifiedGenericsTypeExpression *type_expression);
    llvm::Type *emit_identifier_type_expression(IdentifierTypeExpression *type_expression);
};

llvm::Type *map_liam_type_to_llvm_type(llvm::LLVMContext *context, TypeInfo *type_info);
llvm::Value *get_llvm_true_literal(llvm::LLVMContext *context);
llvm::Value *get_llvm_false_literal(llvm::LLVMContext *context);