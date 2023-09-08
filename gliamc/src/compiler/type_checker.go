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

type LocalSymbolTable struct {
	identifiers map[string]TypeInfo
}

func NewSymbolTable() *LocalSymbolTable {
	return &LocalSymbolTable{identifiers: make(map[string]TypeInfo)}
}

func (self *LocalSymbolTable) AddLocal(identifier string, typeInfo TypeInfo) error {
	_, exists := self.identifiers[identifier]
	if exists {
		return fmt.Errorf("symbol '%v' already exists", identifier)
	}

	self.identifiers[identifier] = typeInfo
	return nil
}

func (self *LocalSymbolTable) GetLocal(identifier string) (TypeInfo, bool) {
	typeInfo, ok := self.identifiers[identifier]
	if !ok {
		return nil, false
	}

	return typeInfo, true
}

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
	FnTypeIndex uint
	Identifier  Token
	ReturnType  TypeInfo
}

func (self *FnTypeInfo) TypeInfoType() TypeInfoType {
	return TI_Fn
}

type TypeChecker struct {
	Ast           AST
	Fns           []*FnTypeInfo
	Types         []TypeInfo
	NameToTypeMap map[string]TypeInfo
}

func NewTypeChecker(ast AST) *TypeChecker {
	return &TypeChecker{
		Ast:           ast,
		Fns:           make([]*FnTypeInfo, 0),
		Types:         make([]TypeInfo, 0),
		NameToTypeMap: make(map[string]TypeInfo),
	}
}

func (self *TypeChecker) AddType(identifier string, typeInfo TypeInfo) {
	self.Types = append(self.Types, typeInfo)
	self.NameToTypeMap[identifier] = typeInfo
}

func (self *TypeChecker) GetType(identifier string) (TypeInfo, bool) {
	typeInfo, ok := self.NameToTypeMap[identifier]
	return typeInfo, ok
}

func (self *TypeChecker) AddFn(fnTypeInfo *FnTypeInfo) {
	index := uint(len(self.Fns))
	self.Fns = append(self.Fns, fnTypeInfo)
	fnTypeInfo.FnTypeIndex = index
}

func (self *TypeChecker) GetFn(identifier string) (*FnTypeInfo, bool) {
	for _, fnTypeInfo := range self.Fns {
		if string(self.Ast.GetTokenSlice(fnTypeInfo.Identifier)) == identifier {
			return fnTypeInfo, true
		}
	}

	return nil, false
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
		FnTypeInfos:      self.Fns,
		Types:            self.Types,
	}, nil
}

func (self *TypeChecker) TypeCheckStatement(statement Statement, symbolTable *LocalSymbolTable) error {
	switch statement.(type) {
	case *ExpressionStatement:
		return self.TypeCheckExpressionStatement(statement.(*ExpressionStatement), symbolTable)
	case *ScopeStatement:
		return self.TypeCheckScopeStatement(statement.(*ScopeStatement), symbolTable)
	case *ReturnStatement:
		return self.TypeCheckReturnStatement(statement.(*ReturnStatement), symbolTable)
	case *IfStatement:
		return self.TypeCheckIfStatement(statement.(*IfStatement), symbolTable)
	case *FnStatement:
		return self.TypeCheckFnStatementBody(statement.(*FnStatement))
	case *StructStatement:
		return self.TypeCheckStructStatement(statement.(*StructStatement))
	case *LetStatement:
		return self.TypeCheckLetStatement(statement.(*LetStatement), symbolTable)
	default:
		log.Fatal("cannot type check this statement yet")
	}

	return nil
}

func (self *TypeChecker) TypeCheckExpressionStatement(statement *ExpressionStatement, symbolTable *LocalSymbolTable) error {
	return self.TypeCheckExpression(statement.expression, symbolTable)
}

func (self *TypeChecker) TypeCheckScopeStatement(statement *ScopeStatement, symbolTable *LocalSymbolTable) error {
	for _, subStatement := range statement.statements {
		err := self.TypeCheckStatement(subStatement, symbolTable)
		if err != nil {
			return err
		}
	}

	return nil
}

func (self *TypeChecker) TypeCheckReturnStatement(statement *ReturnStatement, symbolTable *LocalSymbolTable) error {
	return self.TypeCheckExpression(statement.expression, symbolTable)
}

func (self *TypeChecker) TypeCheckIfStatement(statement *IfStatement, symbolTable *LocalSymbolTable) error {
	err := self.TypeCheckExpression(statement.condition, symbolTable)
	if err != nil {
		return err
	}

	if statement.condition.TypeInfo().TypeInfoType() != TI_Bool {
		return fmt.Errorf("can only pass bool expressions to if")
	}

	err = self.TypeCheckScopeStatement(statement.scope, symbolTable)
	if err != nil {
		return err
	}

	return nil
}

func (self *TypeChecker) TypeCheckFnStatementHeader(statement *FnStatement) error {
	err := self.TypeCheckTypeExpression(statement.ReturnType)
	if err != nil {
		return err
	}

	fnTypeInfo := &FnTypeInfo{
		Identifier: statement.Identifier,
		ReturnType: statement.ReturnType.TypeInfo(),
	}

	self.AddFn(fnTypeInfo)

	return nil
}

func (self *TypeChecker) TypeCheckFnStatementBody(statement *FnStatement) error {
	statement.SymbolTable = NewSymbolTable()
	for _, subStatement := range statement.Body {
		err := self.TypeCheckStatement(subStatement, statement.SymbolTable)
		if err != nil {
			return err
		}
	}

	return nil
}

func (self *TypeChecker) TypeCheckStructStatement(statement *StructStatement) error {
	structName := self.Ast.GetTokenSlice(statement.identifier)
	self.AddType(string(structName), &StructTypeInfo{Name: structName})

	return nil
}

func (self *TypeChecker) TypeCheckLetStatement(statement *LetStatement, symbolTable *LocalSymbolTable) error {
	err := self.TypeCheckTypeExpression(statement.typeExpression)
	if err != nil {
		return err
	}

	err = self.TypeCheckExpression(statement.expression, symbolTable)
	if err != nil {
		return err
	}

	err = symbolTable.AddLocal(string(self.Ast.GetTokenSlice(statement.identifier)), statement.expression.TypeInfo())
	if err != nil {
		return err
	}

	return nil
}

func (self *TypeChecker) TypeCheckExpression(expression Expression, symbolTable *LocalSymbolTable) error {
	switch expression.(type) {
	case *BoolLiteralExpression:
		self.TypeCheckBoolLiteralExpression(expression.(*BoolLiteralExpression))
	case *IdentifierExpression:
		return self.TypeCheckIdentifierExpression(expression.(*IdentifierExpression), symbolTable)
	case *BinaryExpression:
		return self.TypeCheckBinaryExpression(expression.(*BinaryExpression), symbolTable)
	case *GroupExpression:
		return self.TypeCheckGroupExpression(expression.(*GroupExpression), symbolTable)
	case *NumberLiteralExpression:
		self.TypeCheckNumberLiteralExpression(expression.(*NumberLiteralExpression))
	case *CallExpression:
		return self.TypeCheckCallExpression(expression.(*CallExpression), symbolTable)
	default:
		log.Fatal("cannot type check this expression yet")
	}

	return nil
}

func (self *TypeChecker) TypeCheckBoolLiteralExpression(expression *BoolLiteralExpression) {
	expression.typeInfo = self.Types[boolTypeIndex]
}

func (self *TypeChecker) TypeCheckIdentifierExpression(expression *IdentifierExpression, symbolTable *LocalSymbolTable) error {
	identifier := string(self.Ast.GetTokenSlice(expression.identifier))

	var typeInfo TypeInfo = nil

	typeInfo, ok := symbolTable.GetLocal(identifier)
	if ok {
		goto assignType
	}

	typeInfo, ok = self.GetFn(identifier)
	if ok {
		goto assignType
	}

	return fmt.Errorf("identifier '%v' cannot be found", identifier)

assignType:
	expression.typeInfo = typeInfo
	return nil
}

func (self *TypeChecker) TypeCheckBinaryExpression(expression *BinaryExpression, symbolTable *LocalSymbolTable) error {
	err := self.TypeCheckExpression(expression.lhs, symbolTable)
	if err != nil {
		return err
	}

	err = self.TypeCheckExpression(expression.rhs, symbolTable)
	if err != nil {
		return err
	}

	if !EqualTypes(expression.lhs.TypeInfo(), expression.rhs.TypeInfo()) {
		return fmt.Errorf("mismatched types in binary expression")
	}

	return nil
}

func (self *TypeChecker) TypeCheckGroupExpression(expression *GroupExpression, symbolTable *LocalSymbolTable) error {
	err := self.TypeCheckExpression(expression.expression, symbolTable)
	if err != nil {
		return err
	}

	expression.typeInfo = expression.expression.TypeInfo()
	return nil
}

func (self *TypeChecker) TypeCheckNumberLiteralExpression(expression *NumberLiteralExpression) {
	expression.typeInfo = self.Types[intTypeIndex]
}

func (self *TypeChecker) TypeCheckCallExpression(expression *CallExpression, symbolTable *LocalSymbolTable) error {
	err := self.TypeCheckExpression(expression.callee, symbolTable)
	if err != nil {
		return err
	}

	if expression.callee.TypeInfo().TypeInfoType() != TI_Fn {
		return fmt.Errorf("can only call fn types with '()'")
	}

	fnTypeInfo, ok := expression.callee.TypeInfo().(*FnTypeInfo)
	if !ok {
		log.Fatal("fn is always expected here")
	}

	expression.typeInfo = fnTypeInfo.ReturnType
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
