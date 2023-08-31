package compiler

import (
	"fmt"
	"log"
)

type TypeInfoType = uint

const (
	boolTypeIndex = 0
	intTypeIndex  = 1
)

const (
	TI_Fn TypeInfoType = iota
	TI_TypeIndex
)

var (
	BuiltInTypes = [...]Type{
		{
			Name: []rune("bool"),
		},
		{
			Name: []rune("int"),
		},
	}
)

type Type struct {
	Name []rune
}

type TypedAST struct {
	Source           []rune
	TokenBuffer      []TokenData
	FnStatements     []*FnStatement
	StructStatements []*StructStatement
	FnTypeInfos      []*FnTypeInfo
	Types            []Type
}

type TypeInfo interface {
	TypeInfoType() TypeInfoType
}

type TypeIndex struct {
	index uint
}

func (self *TypeIndex) TypeInfoType() TypeInfoType {
	return TI_TypeIndex
}

type FnTypeInfo struct {
	Identifier Token
	ReturnType TypeInfo
}

func (self *FnTypeInfo) TypeInfoType() TypeInfoType {
	return TI_Fn
}

type TypeChecker struct {
	ast         AST
	FnTypeInfos []*FnTypeInfo
	Types       []Type
}

func NewTypeChecker(ast AST) *TypeChecker {
	return &TypeChecker{ast: ast}
}

func (self *TypeChecker) TypeCheck() (TypedAST, error) {

	// add all the builtin types to the type table
	for _, builtInTypo := range BuiltInTypes {
		self.Types = append(self.Types, builtInTypo)
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
		Types:            self.Types,
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
	for _, subStatement := range statement.statements {
		err := self.TypeCheckStatement(subStatement)
		if err != nil {
			return err
		}
	}

	return nil
}

func (self *TypeChecker) TypeCheckReturnStatement(statement *ReturnStatement) error {
	return self.TypeCheckExpression(statement.expression)
}

func (self *TypeChecker) TypeCheckIfStatement(statement *IfStatement) error {
	return nil
}

func (self *TypeChecker) TypeCheckFnStatementHeader(statement *FnStatement) error {
	err := self.TypeCheckTypeExpression(statement.returnType)
	if err != nil {
		return err
	}

	fnTypeInfo := &FnTypeInfo{
		Identifier: statement.identifier,
		ReturnType: statement.returnType.TypeInfo(),
	}

	self.FnTypeInfos = append(self.FnTypeInfos, fnTypeInfo)
	return nil
}

func (self *TypeChecker) TypeCheckFnStatementBody(statement *FnStatement) error {
	return self.TypeCheckScopeStatement(statement.body)
}

func (self *TypeChecker) TypeCheckStructStatement(statement *StructStatement) error {
	structName := self.ast.GetTokenSlice(statement.identifier)
	newType := Type{Name: structName}
	self.Types = append(self.Types, newType)

	// TODO in the future we need to make sure there are no conflicting type names
	return nil
}

func (self *TypeChecker) TypeCheckExpression(expression Expression) error {
	switch expression.(type) {
	case *BoolLiteralExpression:
		self.TypeCheckBoolLiteralExpression(expression.(*BoolLiteralExpression))
		return nil
	case *IdentifierExpression:
		return self.TypeCheckIdentifierExpression(expression.(*IdentifierExpression))
	case *BinaryExpression:
		return self.TypeCheckBinaryExpression(expression.(*BinaryExpression))
	case *GroupExpression:
		return self.TypeCheckGroupExpression(expression.(*GroupExpression))
	case *NumberLiteralExpression:
		self.TypeCheckNumberLiteralExpression(expression.(*NumberLiteralExpression))
		return nil
	default:
		log.Fatal("cannot type check this expression yet")
	}

	return nil
}

func (self *TypeChecker) TypeCheckBoolLiteralExpression(expression *BoolLiteralExpression) {
	expression.typeInfo = &TypeIndex{index: boolTypeIndex}
}

func (self *TypeChecker) TypeCheckIdentifierExpression(expression *IdentifierExpression) error {
	return nil
}

func (self *TypeChecker) TypeCheckBinaryExpression(expression *BinaryExpression) error {
	err := self.TypeCheckExpression(expression.lhs)
	if err != nil {
		return err
	}

	err = self.TypeCheckExpression(expression.rhs)
	if err != nil {
		return err
	}

	if !EqualTypes(expression.lhs.TypeInfo(), expression.rhs.TypeInfo()) {
		return fmt.Errorf("mismatched types in binary expression")
	}

	return nil
}

func (self *TypeChecker) TypeCheckGroupExpression(expression *GroupExpression) error {
	err := self.TypeCheckExpression(expression.expression)
	if err != nil {
		return err
	}

	expression.typeInfo = expression.expression.TypeInfo()
	return nil
}

func (self *TypeChecker) TypeCheckNumberLiteralExpression(expression *NumberLiteralExpression) {
	expression.typeInfo = &TypeIndex{index: intTypeIndex}
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
	identifier := self.ast.GetTokenSlice(typeExpression.identifier)
	for index, t := range self.Types {
		if SliceEqual(&identifier, &t.Name) {
			typeExpression.typeInfo = &TypeIndex{index: uint(index)}
			return nil
		}
	}

	return fmt.Errorf("unknown type '%v' in type expression", string(identifier))
}

func EqualTypes(left TypeInfo, right TypeInfo) bool {

	if left.TypeInfoType() != right.TypeInfoType() {
		return false
	}

	switch left.TypeInfoType() {
	case TI_TypeIndex:
		typeIndexLeft := left.(*TypeIndex)
		typeIndexRight := right.(*TypeIndex)

		return typeIndexLeft.index == typeIndexRight.index
	case TI_Fn:
		fnLeft := left.(*FnTypeInfo)
		fnRight := right.(*FnTypeInfo)

		return fnLeft.Identifier == fnRight.Identifier
	default:
		log.Fatal("unsupported type info type")
		return false
	}
}

func isSubType[T any](i interface{}) bool {
	_, ok := i.(T)
	return ok
}

func SliceEqual[T comparable](a *[]T, b *[]T) bool {
	if len(*a) != len(*b) {
		return false
	}

	for index, c := range *a {
		if (*b)[index] != c {
			return false
		}
	}

	return true
}
