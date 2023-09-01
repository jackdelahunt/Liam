package compiler

import (
	"fmt"
	"log"
)

type TypeInfoType = uint

const (
	TI_Fn TypeInfoType = iota
	TI_Bool
	TI_Int
	TI_Struct
)

const (
	boolTypeIndex = 0
	intTypeIndex  = 1
)

var (
	BuiltInTypes = [...]struct {
		identifier string
		typeInfo   TypeInfo
	}{
		{
			identifier: "bool",
			typeInfo:   &BoolTypeInfo{},
		},
		{
			identifier: "int",
			typeInfo:   &IntTypeInfo{},
		},
	}
)

type TypedAST struct {
	Source           []rune
	TokenBuffer      []TokenData
	FnStatements     []*FnStatement
	StructStatements []*StructStatement
	FnTypeInfos      []*FnTypeInfo
	Types            []TypeInfo
	NameToTypeMap    map[string]TypeInfo
}

type TypeInfo interface {
	TypeInfoType() TypeInfoType
}

type IntTypeInfo struct {
}

func (self *IntTypeInfo) TypeInfoType() TypeInfoType {
	return TI_Int
}

type BoolTypeInfo struct {
}

func (self *BoolTypeInfo) TypeInfoType() TypeInfoType {
	return TI_Bool
}

type StructTypeInfo struct {
	Name []rune
}

func (self *StructTypeInfo) TypeInfoType() TypeInfoType {
	return TI_Struct
}

type FnTypeInfo struct {
	Identifier Token
	ReturnType TypeInfo
}

func (self *FnTypeInfo) TypeInfoType() TypeInfoType {
	return TI_Fn
}

type TypeChecker struct {
	Ast           AST
	FnTypeInfos   []*FnTypeInfo
	Types         []TypeInfo
	NameToTypeMap map[string]TypeInfo
}

func NewTypeChecker(ast AST) *TypeChecker {
	return &TypeChecker{
		Ast:           ast,
		FnTypeInfos:   make([]*FnTypeInfo, 0),
		Types:         make([]TypeInfo, 0),
		NameToTypeMap: make(map[string]TypeInfo),
	}
}

func (self *TypeChecker) AddType(identifier string, typeInfo TypeInfo) {
	self.Types = append(self.Types, typeInfo)
	self.NameToTypeMap[identifier] = typeInfo
}

func (self *TypeChecker) TypeCheck() (TypedAST, error) {

	// add all the builtin types to the type table
	for _, builtInTypo := range BuiltInTypes {
		self.AddType(builtInTypo.identifier, builtInTypo.typeInfo)
	}

	for _, statement := range self.Ast.StructStatements {
		err := self.TypeCheckStructStatement(statement)
		if err != nil {
			return TypedAST{}, err
		}
	}

	for _, statement := range self.Ast.FnStatements {
		err := self.TypeCheckFnStatementHeader(statement)
		if err != nil {
			return TypedAST{}, err
		}
	}

	for _, statement := range self.Ast.FnStatements {
		err := self.TypeCheckFnStatementBody(statement)
		if err != nil {
			return TypedAST{}, err
		}
	}

	return TypedAST{
		Source:           self.Ast.Source,
		TokenBuffer:      self.Ast.TokenBuffer,
		FnStatements:     self.Ast.FnStatements,
		StructStatements: self.Ast.StructStatements,
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
	return self.TypeCheckExpression(statement.expression)
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
	err := self.TypeCheckExpression(statement.condition)
	if err != nil {
		return err
	}

	if statement.condition.TypeInfo().TypeInfoType() != TI_Bool {
		return fmt.Errorf("can only pass bool expressions to if")
	}

	err = self.TypeCheckScopeStatement(statement.scope)
	if err != nil {
		return err
	}

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
	structName := self.Ast.GetTokenSlice(statement.identifier)
	self.AddType(string(structName), &StructTypeInfo{Name: structName})

	return nil
}

func (self *TypeChecker) TypeCheckExpression(expression Expression) error {
	switch expression.(type) {
	case *BoolLiteralExpression:
		self.TypeCheckBoolLiteralExpression(expression.(*BoolLiteralExpression))
	case *IdentifierExpression:
		return self.TypeCheckIdentifierExpression(expression.(*IdentifierExpression))
	case *BinaryExpression:
		return self.TypeCheckBinaryExpression(expression.(*BinaryExpression))
	case *GroupExpression:
		return self.TypeCheckGroupExpression(expression.(*GroupExpression))
	case *NumberLiteralExpression:
		self.TypeCheckNumberLiteralExpression(expression.(*NumberLiteralExpression))
	default:
		log.Fatal("cannot type check this expression yet")
	}

	return nil
}

func (self *TypeChecker) TypeCheckBoolLiteralExpression(expression *BoolLiteralExpression) {
	expression.typeInfo = self.Types[boolTypeIndex]
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
	expression.typeInfo = self.Types[intTypeIndex]
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
	identifier := self.Ast.GetTokenSlice(typeExpression.identifier)
	typeInfo, ok := self.NameToTypeMap[string(identifier)]
	if !ok {
		return fmt.Errorf("use of undeclared type %v", string(identifier))
	}

	typeExpression.typeInfo = typeInfo
	return nil
}

func EqualTypes(left TypeInfo, right TypeInfo) bool {

	if left.TypeInfoType() != right.TypeInfoType() {
		return false
	}

	switch left.TypeInfoType() {
	case TI_Bool:
		return true
	case TI_Int:
		return true
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
