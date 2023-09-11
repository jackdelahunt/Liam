package compiler

type AST struct {
	Source      []rune
	TokenBuffer []TokenData
	Statements  []Statement
}

/*
	   ============
		Statements
	   ============
*/
type Statement interface {
}

type ScopeStatement struct {
	statements  []Statement
	symbolTable LocalSymbolTable
}

type ReturnStatement struct {
	expression Expression
}

type FnStatement struct {
	Identifier  Token
	ReturnType  TypeExpression
	Body        []Statement
	SymbolTable *LocalSymbolTable
}
type StructStatement struct {
	Identifier Token
}

/*
	   ============
		Expressions
	   ============
*/
type Expression interface {
	TypeInfo() TypeInfo
}

type BoolLiteralExpression struct {
	value    Token
	typeInfo TypeInfo
}

func (self *BoolLiteralExpression) TypeInfo() TypeInfo {
	return self.typeInfo
}

type IdentifierExpression struct {
	identifier Token
	typeInfo   TypeInfo
}

func (self *IdentifierExpression) TypeInfo() TypeInfo {
	return self.typeInfo
}

type BinaryExpression struct {
	lhs          Expression
	operator     Token
	rhs          Expression
	operatorType OperatorType
	typeInfo     TypeInfo
}

func (self *BinaryExpression) TypeInfo() TypeInfo {
	return self.typeInfo
}

type CallExpression struct {
	callee   Expression
	typeInfo TypeInfo
}

func (self *CallExpression) TypeInfo() TypeInfo {
	return self.typeInfo
}

type NumberLiteralExpression struct {
	number   Token
	typeInfo TypeInfo
}

func (self *NumberLiteralExpression) TypeInfo() TypeInfo {
	return self.typeInfo
}

/*
	   ============
		Type Expressions
	   ============
*/
type TypeExpression interface {
	TypeInfo() TypeInfo
}

type IdentifierTypeExpression struct {
	identifier Token
	typeInfo   TypeInfo
}

func (self *IdentifierTypeExpression) TypeInfo() TypeInfo {
	return self.typeInfo
}
