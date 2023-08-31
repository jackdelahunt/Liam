package compiler

import (
	"log"
)

var (
	BuiltInTypes = [...]BuiltInTypeInfo{
		{
			Name: "bool",
		},
	}
)

type TypedAST struct {
	Source           []rune
	TokenBuffer      []TokenData
	FnStatements     []*FnStatement
	StructStatements []*StructStatement
	FnTypeInfos      []*FnInfo
	TypeTypeInfos    []TypeInfo
}

type TypeInfo interface {
}

type BuiltInTypeInfo struct {
	Name string
}

type FnInfo struct {
	Name       string
	ReturnType TypeInfo
}

type TypeChecker struct {
	ast           AST
	FnTypeInfos   []*FnInfo
	TypeTypeInfos []TypeInfo
}

func NewTypeChecker(ast AST) *TypeChecker {
	return &TypeChecker{ast: ast}
}

func (self *TypeChecker) TypeCheck() (TypedAST, error) {

	// add all the builtin types to the type table
	for _, builtInTypo := range BuiltInTypes {
		self.TypeTypeInfos = append(self.TypeTypeInfos, builtInTypo)
	}

	for _, statement := range self.ast.StructStatements {
		err := self.TypeCheckStructStatement(statement)
		if err != nil {
			return TypedAST{}, err
		}
	}

	for _, statement := range self.ast.FnStatements {
		err := self.TypeCheckFnStatementHeader(statement)
		if err != nil {
			return TypedAST{}, err
		}
	}

	for _, statement := range self.ast.FnStatements {
		err := self.TypeCheckFnStatementBody(statement)
		if err != nil {
			return TypedAST{}, err
		}
	}

	return TypedAST{
		Source:           self.ast.Source,
		TokenBuffer:      self.ast.TokenBuffer,
		FnStatements:     self.ast.FnStatements,
		StructStatements: self.ast.StructStatements,
		FnTypeInfos:      self.FnTypeInfos,
		TypeTypeInfos:    self.TypeTypeInfos,
	}, nil
}

func (self *TypeChecker) TypeCheckStatement(statement Statement) error {
	switch statement.(type) {
	case *ExpressionStatement:
		return self.TypeCheckExpressionStatement(statement.(*ExpressionStatement))
	case *ScopeStatement:
		return self.TypeCheckScopeStatement(statement.(*ScopeStatement))
	case *ReturnStatement:
		return self.TypeCheckReturnStatement(statement.(*ReturnStatement))
	case *IfStatement:
		return self.TypeCheckIfStatement(statement.(*IfStatement))
	case *FnStatement:
		return self.TypeCheckFnStatementBody(statement.(*FnStatement))
	case *StructStatement:
		return self.TypeCheckStructStatement(statement.(*StructStatement))
	default:
		log.Fatal("cannot type check this statement yet")
	}

	return nil
}

func (self *TypeChecker) TypeCheckExpressionStatement(statement *ExpressionStatement) error {
	return nil
}

func (self *TypeChecker) TypeCheckScopeStatement(statement *ScopeStatement) error {
	return nil
}

func (self *TypeChecker) TypeCheckReturnStatement(statement *ReturnStatement) error {
	return nil
}

func (self *TypeChecker) TypeCheckIfStatement(statement *IfStatement) error {
	return nil
}

func (self *TypeChecker) TypeCheckFnStatementHeader(statement *FnStatement) error {
	return nil
}

func (self *TypeChecker) TypeCheckFnStatementBody(statement *FnStatement) error {
	return nil
}

func (self *TypeChecker) TypeCheckStructStatement(statement *StructStatement) error {
	return nil
}

func (self *TypeChecker) TypeCheckExpression(expression Expression) error {
	switch expression.(type) {
	case *BoolLiteralExpression:
		return self.TypeCheckBoolLiteralExpression(expression.(*BoolLiteralExpression))
	case *IdentifierExpression:
		return self.TypeCheckIdentifierExpression(expression.(*IdentifierExpression))
	case *BinaryExpression:
		return self.TypeCheckBinaryExpression(expression.(*BinaryExpression))
	case *GroupExpression:
		return self.TypeCheckGroupExpression(expression.(*GroupExpression))
	case *NumberLiteralExpression:
		return self.TypeCheckNumberLiteralExpression(expression.(*NumberLiteralExpression))
	default:
		log.Fatal("cannot type check this expression yet")
	}

	return nil
}

func (self *TypeChecker) TypeCheckBoolLiteralExpression(expression *BoolLiteralExpression) error {
	return nil
}

func (self *TypeChecker) TypeCheckIdentifierExpression(expression *IdentifierExpression) error {
	return nil
}

func (self *TypeChecker) TypeCheckBinaryExpression(expression *BinaryExpression) error {
	return nil
}

func (self *TypeChecker) TypeCheckGroupExpression(expression *GroupExpression) error {
	return nil
}

func (self *TypeChecker) TypeCheckNumberLiteralExpression(expression *NumberLiteralExpression) error {
	return nil
}

func (self *TypeChecker) TypeCheckTypeExpression(typeExpression TypeExpression) error {
	switch typeExpression.(type) {
	case *IdentifierTypeExpression:
		return self.TypeCheckIdentifierTypeExpression(typeExpression.(*IdentifierTypeExpression))
	default:
		log.Fatal("cannot type check this type expression yet")
	}

	return nil
}

func (self *TypeChecker) TypeCheckIdentifierTypeExpression(typeExpression *IdentifierTypeExpression) error {
	return nil
}
