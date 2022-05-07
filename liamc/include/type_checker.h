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

struct TypedStatement;
struct TypedLetStatement;
struct TypedScopeStatement;
struct TypedStructStatement;
struct TypedFnStatement;
struct TypedLoopStatement;
struct TypedInsertStatement;
struct TypedReturnStatement;
struct TypedBreakStatement;
struct TypedExpressionStatement;
struct TypedAssigmentStatement;
struct TypedIntLiteralExpression;
struct TypedStringLiteralExpression;
struct TypedExpression;
struct TypedIdentifierExpression;
struct TypedFile;

typedef std::vector<std::tuple<Token, TypedExpression*>> Typed_CSV;

enum TypeInfoType {
	VOID,
	INT,
	STRING,
	FN,
	STRUCT,
	POINTER
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

struct FnTypeInfo : TypeInfo {
	TypeInfo* return_type;
	std::vector<TypeInfo*> args;
};

struct StructTypeInfo : TypeInfo {
	std::vector<std::tuple<std::string, TypeInfo*>> members;
};

struct SymbolTable {
	std::map<std::string, TypeInfo*> builtin_type_table;
	std::map<std::string, TypeInfo*> type_table;
	std::map<std::string, TypeInfo*> identifier_table;

	SymbolTable();
	 
	void add_type(Token type, TypeInfo* type_info);
	void add_identifier(Token identifier, TypeInfo* type_info);
};

struct TypedStatement {
	StatementType statement_type;
	virtual void print();
};

struct TypedLetStatement: TypedStatement {
	Token identifier;
	TypedExpression* type_expression;
	TypedExpression* expression;

	TypedLetStatement(Token identifier, TypedExpression* type_expression, TypedExpression* expression);
};

struct TypedScopeStatement : TypedStatement {
	std::vector<TypedStatement*> statements;

	TypedScopeStatement(std::vector<TypedStatement*> statements);
};

struct TypedStructStatement : TypedStatement {
	Token identifier;
	Typed_CSV members;

	TypedStructStatement(Token identifier, Typed_CSV members);
};

struct TypedFnStatement : TypedStatement {
	Token identifier;
	Typed_CSV params;
	TypedExpression* return_type;
	TypedScopeStatement* body;

	TypedFnStatement(Token identifier, Typed_CSV params, TypedExpression* return_type, TypedScopeStatement* body);
};

struct TypedLoopStatement : TypedStatement {
	Token identifier;
	TypedScopeStatement* body;

	TypedLoopStatement(Token identifier, TypedScopeStatement* body);
};


struct TypedInsertStatement : TypedStatement {
	TypedStringLiteralExpression* code;

	TypedInsertStatement(TypedStringLiteralExpression* code);
};

struct TypedReturnStatement : TypedStatement {
	TypedExpression* expression;

	TypedReturnStatement(TypedExpression* epxression);
};

struct TypedBreakStatement : TypedStatement {
	Token identifier;

	TypedBreakStatement(Token identifier);
};

struct TypedImportStatement : TypedStatement {
	TypedExpression* file;

	TypedImportStatement(TypedExpression* file);
};

struct TypedExpressionStatement : TypedStatement {
	TypedExpression* expression;

	TypedExpressionStatement(TypedExpression* expression);
};

struct TypedAssigmentStatement : TypedStatement {
	Token identifier;
	TypedExpression* assigned_to;

	TypedAssigmentStatement(Token identifier, TypedExpression* assigned_to);
};

struct TypedExpression {
    ExpressionType type;
    TypeInfo* type_info;
	virtual void print();
};

struct TypedBinaryExpression : TypedExpression {
	TypedExpression* left;
	Token op;
	TypedExpression* right;

	TypedBinaryExpression(TypedExpression* left, Token op, TypedExpression* right);
};

struct TypedUnaryExpression : TypedExpression {
	Token op;
	TypedExpression* expression;

	TypedUnaryExpression(TypedExpression* expression, Token op);
};

struct TypedIntLiteralExpression : TypedExpression {
	Token token;

	TypedIntLiteralExpression(Token token);
};

struct TypedCallExpression : TypedExpression {
	// this is an expression but it must be a identifier
	TypedIdentifierExpression* identifier;
	std::vector<TypedExpression*> args;

	TypedCallExpression(TypedIdentifierExpression* identifier, std::vector<TypedExpression*> args);
};

struct TypedIdentifierExpression : TypedExpression {
	Token identifier;

	TypedIdentifierExpression(Token identifier, TypeInfo* type_info);
};

struct TypedStringLiteralExpression : TypedExpression {
	Token token;

	TypedStringLiteralExpression(Token token);
};

struct TypedGetExpression : TypedExpression {
	// this is an expression but it must be a identifier
	TypedExpression* expression;
	Token member;

	TypedGetExpression(TypedExpression* expression, Token member);
};

struct TypedNewExpression : TypedExpression {
	Token identifier;
	std::vector<TypedExpression*> expressions;

	TypedNewExpression(Token identifier, std::vector<TypedExpression*> expressions);
};

struct TypedFile {
	std::vector<TypedStatement*> statements;
};

struct TypeChecker {
	SymbolTable symbol_table;
	TypedFile root;

	TypeChecker();

	void type_file(File* file);
	TypedStatement* type_statement(Statement* statement, SymbolTable* symbol_table);
	TypedInsertStatement* type_insert_statement(InsertStatement* statement, SymbolTable* symbol_table);
	TypedReturnStatement* type_return_statement(ReturnStatement* statement, SymbolTable* symbol_table);
	TypedBreakStatement* type_break_statement(BreakStatement* statement, SymbolTable* symbol_table);
	TypedImportStatement* type_import_statement(ImportStatement* statement, SymbolTable* symbol_table);
	TypedLetStatement* type_let_statement(LetStatement* statement, SymbolTable* symbol_table);
	TypedScopeStatement* type_scope_statement(ScopeStatement* statement, SymbolTable* symbol_table, bool copy_symbol_table = true);
	TypedFnStatement* type_fn_statement(FnStatement* statement, SymbolTable* symbol_table);
	TypedLoopStatement* type_loop_statement(LoopStatement* statement, SymbolTable* symbol_table);
	TypedStructStatement* type_struct_statement(StructStatement* statement, SymbolTable* symbol_table);
	TypedAssigmentStatement* type_assigment_statement(AssigmentStatement* statement, SymbolTable* symbol_table);
	TypedExpressionStatement* type_expression_statement(ExpressionStatement* statement, SymbolTable* symbol_table);

	TypedExpression* type_expression(Expression* expression, SymbolTable* symbol_table);
	TypedBinaryExpression* type_binary_expression(BinaryExpression* expression, SymbolTable* symbol_table);
	TypedStringLiteralExpression* type_string_literal_expression(StringLiteralExpression* expression, SymbolTable* symbol_table);
	TypedIntLiteralExpression* type_int_literal_expression(IntLiteralExpression* expression, SymbolTable* symbol_table);
	TypedUnaryExpression* type_unary_expression(UnaryExpression* expression, SymbolTable* symbol_table);
	TypedCallExpression* type_call_expression(CallExpression* expression, SymbolTable* symbol_table);
	TypedIdentifierExpression* type_identifier_expression(IdentifierExpression* expression, SymbolTable* symbol_table);
	TypedGetExpression* type_get_expression(GetExpression* expression, SymbolTable* symbol_table);
	TypedNewExpression* type_new_expression(NewExpression* expression, SymbolTable* symbol_table);
};

bool type_match(TypeInfo* a, TypeInfo* b);
