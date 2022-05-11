#pragma once
#include "parser.h"
#include <map>

struct Statement;
struct LetStatement;
struct ScopeStatement;
struct StructStatement;
struct FnStatement;
struct LoopStatement;
struct InsertStatement;
struct ReturnStatement;
struct BreakStatement;
struct ExpressionStatement;
struct AssigmentStatement;
struct IntLiteralExpression;
struct StringLiteralExpression;
struct Expression;
struct File;

struct TypeCheckedStatement;
struct TypeCheckedLetStatement;
struct TypeCheckedScopeStatement;
struct TypeCheckedStructStatement;
struct TypeCheckedIdentifierExpression;
struct TypeCheckedFnStatement;
struct TypeCheckedLoopStatement;
struct TypeCheckedInsertStatement;
struct TypeCheckedReturnStatement;
struct TypeCheckedBreakStatement;
struct TypeCheckedExpressionStatement;
struct TypeCheckedAssigmentStatement;

struct TypeCheckedIntLiteralExpression;
struct TypeCheckedStringLiteralExpression;
struct TypeCheckedExpression;

struct TypeCheckedTypeExpression;
struct TypeCheckedIdentifierTypeExpression;
struct TypeCheckedPointerTypeExpression;

struct TypedFile;

typedef std::vector<std::tuple<Token, TypeCheckedTypeExpression*>> Typed_CSV;

enum TypeInfoType {
	VOID,
	INT,
	STRING,
	FN,
	STRUCT,
    POINTER,
    ARRAY,
};

struct TypeInfo {
	TypeInfoType type;
};

struct VoidTypeInfo : TypeInfo {
};

struct IntTypeInfo : TypeInfo {
	bool is_signed;
	size_t size;
};

struct StringTypeInfo : TypeInfo {
};

struct PointerTypeInfo : TypeInfo {
	TypeInfo* to;
};

struct TypeTypeInfo : TypeInfo {
};

struct ArrayTypeInfo : TypeInfo {
    TypeInfo* array_type;
};

struct FnTypeInfo : TypeInfo {
	TypeInfo* return_type;
	std::vector<TypeInfo*> args;
};

struct StructTypeInfo : TypeInfo {
	std::vector<std::tuple<std::string, TypeInfo*>> members;
};

struct SymbolTable {
	std::map<std::string, TypeInfo*> builtin_type_table; // u64 string...
	std::map<std::string, TypeInfo*> type_table;         // structs
	std::map<std::string, TypeInfo*> identifier_table;   // variables or funcs

	SymbolTable();
	 
	void add_type(Token type, TypeInfo* type_info);
	void add_identifier(Token identifier, TypeInfo* type_info);
    TypeInfo* get_type(Token* identifier);
};

struct TypeCheckedStatement {
	StatementType statement_type;
	virtual void print();
};

struct TypeCheckedLetStatement: TypeCheckedStatement {
	Token identifier;
    TypeCheckedTypeExpression* type_expression;
	TypeCheckedExpression* expression;

	TypeCheckedLetStatement(Token identifier, TypeCheckedTypeExpression* type_expression, TypeCheckedExpression* expression);
};

struct TypeCheckedScopeStatement : TypeCheckedStatement {
	std::vector<TypeCheckedStatement*> statements;

    TypeCheckedScopeStatement(std::vector<TypeCheckedStatement*> statements);
};

struct TypeCheckedStructStatement : TypeCheckedStatement {
	Token identifier;
	Typed_CSV members;

    TypeCheckedStructStatement(Token identifier, Typed_CSV members);
};

struct TypeCheckedFnStatement : TypeCheckedStatement {
	Token identifier;
	Typed_CSV params;
	TypeCheckedTypeExpression* return_type;
    TypeCheckedScopeStatement* body;

    TypeCheckedFnStatement(Token identifier, Typed_CSV params, TypeCheckedTypeExpression* return_type, TypeCheckedScopeStatement* body);
};

struct TypeCheckedLoopStatement : TypeCheckedStatement {
	Token identifier;
    TypeCheckedScopeStatement* body;

    TypeCheckedLoopStatement(Token identifier, TypeCheckedScopeStatement* body);
};


struct TypeCheckedInsertStatement : TypeCheckedStatement {
    TypeCheckedStringLiteralExpression* code;

    TypeCheckedInsertStatement(TypeCheckedStringLiteralExpression* code);
};

struct TypeCheckedReturnStatement : TypeCheckedStatement {
	TypeCheckedExpression* expression;

    TypeCheckedReturnStatement(TypeCheckedExpression* epxression);
};

struct TypeCheckedBreakStatement : TypeCheckedStatement {
	Token identifier;

    TypeCheckedBreakStatement(Token identifier);
};

struct TypeCheckedImportStatement : TypeCheckedStatement {
	TypeCheckedExpression* file;

    TypeCheckedImportStatement(TypeCheckedExpression* file);
};

struct TypeCheckedExpressionStatement : TypeCheckedStatement {
	TypeCheckedExpression* expression;

    TypeCheckedExpressionStatement(TypeCheckedExpression* expression);
};

struct TypeCheckedAssigmentStatement : TypeCheckedStatement {
	Token identifier;
	TypeCheckedExpression* assigned_to;

    TypeCheckedAssigmentStatement(Token identifier, TypeCheckedExpression* assigned_to);
};

struct TypeCheckedExpression {
    ExpressionType type;
    TypeInfo* type_info;
	virtual void print();
};

struct TypeCheckedBinaryExpression : TypeCheckedExpression {
	TypeCheckedExpression* left;
	Token op;
	TypeCheckedExpression* right;

    TypeCheckedBinaryExpression(TypeCheckedExpression* left, Token op, TypeCheckedExpression* right);
};

struct TypeCheckedUnaryExpression : TypeCheckedExpression {
	Token op;
	TypeCheckedExpression* expression;

    TypeCheckedUnaryExpression(TypeCheckedExpression* expression, Token op);
};

struct TypeCheckedIntLiteralExpression : TypeCheckedExpression {
	Token token;

    TypeCheckedIntLiteralExpression(Token token);
};

struct TypeCheckedCallExpression : TypeCheckedExpression {
	// this is an expression but it must be a identifier
    TypeCheckedIdentifierExpression* identifier;
	std::vector<TypeCheckedExpression*> args;

    TypeCheckedCallExpression(TypeCheckedIdentifierExpression* identifier, std::vector<TypeCheckedExpression*> args);
};

struct TypeCheckedIdentifierExpression : TypeCheckedExpression {
	Token identifier;

    TypeCheckedIdentifierExpression(Token identifier, TypeInfo* type_info);
};

struct TypeCheckedStringLiteralExpression : TypeCheckedExpression {
	Token token;

    TypeCheckedStringLiteralExpression(Token token);
};

struct TypeCheckedGetExpression : TypeCheckedExpression {
	// this is an expression but it must be a identifier
	TypeCheckedExpression* expression;
	Token member;

    TypeCheckedGetExpression(TypeCheckedExpression* expression, Token member);
};

struct TypeCheckedNewExpression : TypeCheckedExpression {
	Token identifier;
	std::vector<TypeCheckedExpression*> expressions;

    TypeCheckedNewExpression(Token identifier, std::vector<TypeCheckedExpression*> expressions);
};

struct TypeCheckedArrayExpression : TypeCheckedExpression {
    std::vector<TypeCheckedExpression*> expressions;

    TypeCheckedArrayExpression(std::vector<TypeCheckedExpression*> expressions);
};

struct TypeCheckedTypeExpression {
    TypeExpressionType type;
    TypeInfo* type_info = nullptr;
    virtual void print();
};

struct TypeCheckedIdentifierTypeExpression : TypeCheckedTypeExpression {
    Token identifier;

    TypeCheckedIdentifierTypeExpression(Token identifier);
};

struct TypeCheckedPointerTypeExpression : TypeCheckedTypeExpression {
    TypeCheckedTypeExpression* pointer_of;

    TypeCheckedPointerTypeExpression(TypeCheckedTypeExpression* pointer_of);
};

struct TypeCheckedArrayTypeExpression : TypeCheckedTypeExpression {
    TypeCheckedTypeExpression* array_of;

    TypeCheckedArrayTypeExpression(TypeCheckedTypeExpression* array_of);
};

struct TypedFile {
	std::vector<TypeCheckedStatement*> statements;
};

struct TypeChecker {
	SymbolTable symbol_table;
	TypedFile root;

	TypeChecker();

	void type_file(File* file);
	TypeCheckedStatement* type_check_statement(Statement* statement, SymbolTable* symbol_table);
    TypeCheckedInsertStatement* type_check_insert_statement(InsertStatement* statement, SymbolTable* symbol_table);
    TypeCheckedReturnStatement* type_check_return_statement(ReturnStatement* statement, SymbolTable* symbol_table);
    TypeCheckedBreakStatement* type_check_break_statement(BreakStatement* statement, SymbolTable* symbol_table);
    TypeCheckedImportStatement* type_check_import_statement(ImportStatement* statement, SymbolTable* symbol_table);
	TypeCheckedLetStatement* type_check_let_statement(LetStatement* statement, SymbolTable* symbol_table);
    TypeCheckedScopeStatement* type_check_scope_statement(ScopeStatement* statement, SymbolTable* symbol_table, bool copy_symbol_table = true);
    TypeCheckedFnStatement* type_check_fn_statement(FnStatement* statement, SymbolTable* symbol_table);
    TypeCheckedLoopStatement* type_check_loop_statement(LoopStatement* statement, SymbolTable* symbol_table);
    TypeCheckedStructStatement* type_check_struct_statement(StructStatement* statement, SymbolTable* symbol_table);
    TypeCheckedAssigmentStatement* type_check_assigment_statement(AssigmentStatement* statement, SymbolTable* symbol_table);
    TypeCheckedExpressionStatement* type_check_expression_statement(ExpressionStatement* statement, SymbolTable* symbol_table);

	TypeCheckedExpression* type_check_expression(Expression* expression, SymbolTable* symbol_table);
    TypeCheckedIdentifierExpression* type_check_identifier_expression(IdentifierExpression* expression, SymbolTable* symbol_table);
    TypeCheckedBinaryExpression* type_check_binary_expression(BinaryExpression* expression, SymbolTable* symbol_table);
    TypeCheckedStringLiteralExpression* type_check_string_literal_expression(StringLiteralExpression* expression, SymbolTable* symbol_table);
    TypeCheckedIntLiteralExpression* type_check_int_literal_expression(IntLiteralExpression* expression, SymbolTable* symbol_table);
    TypeCheckedUnaryExpression* type_check_unary_expression(UnaryExpression* expression, SymbolTable* symbol_table);
    TypeCheckedCallExpression* type_check_call_expression(CallExpression* expression, SymbolTable* symbol_table);
    TypeCheckedGetExpression* type_check_get_expression(GetExpression* expression, SymbolTable* symbol_table);
    TypeCheckedNewExpression* type_check_new_expression(NewExpression* expression, SymbolTable* symbol_table);
    TypeCheckedArrayExpression* type_check_array_expression(ArrayExpression* expression, SymbolTable* symbol_table);

    TypeCheckedTypeExpression* type_check_type_expression(TypeExpression* type_expression, SymbolTable* symbol_table);
    TypeCheckedIdentifierTypeExpression* type_check_identifier_type_expression(IdentifierTypeExpression* type_expression, SymbolTable* symbol_table);
    TypeCheckedPointerTypeExpression* type_check_pointer_type_expression(PointerTypeExpression* type_expression, SymbolTable* symbol_table);
    TypeCheckedArrayTypeExpression* type_check_array_type_expression(ArrayTypeExpression* type_expression, SymbolTable* symbol_table);
};

bool type_match(TypeInfo* a, TypeInfo* b);
