package compiler

import (
	"strconv"
	"strings"
)

type CCodeGen struct {
	statements *[]Statement
}

func NewCCodeGen(statements *[]Statement) *CCodeGen {
	return &CCodeGen{statements: statements}
}

func (self *CCodeGen) CodeGen() string {
	total := strings.Builder{}
	for _, statement := range *self.statements {
		total.WriteString(self.CodeGenStatement(statement))
		total.WriteString("\n")
	}

	return total.String()
}

func (self *CCodeGen) CodeGenStatement(statement Statement) string {
	switch statement.(type) {
	case *ExpressionStatement:
		return self.CodeGenExpressionStatement(statement.(*ExpressionStatement))
	case *ScopeStatement:
		return self.CodeGenScopeStatement(statement.(*ScopeStatement))
	case *ReturnStatement:
		return self.CodeGenReturnStatement(statement.(*ReturnStatement))
	case *IfStatement:
		return self.CodeGenIfStatement(statement.(*IfStatement))
	case *FnStatement:
		return self.CodeGenFnStatement(statement.(*FnStatement))
	default:
		panic("cannot code gen this statement, not implemented!!")
	}
}

func (self *CCodeGen) CodeGenExpressionStatement(statement *ExpressionStatement) string {
	return self.CodeGenExpression(statement.expression)
}

func (self *CCodeGen) CodeGenScopeStatement(statement *ScopeStatement) string {
	builder := strings.Builder{}
	builder.WriteString("{\n")
	for _, statement := range statement.statements {
		builder.WriteString(self.CodeGenStatement(statement))
	}
	builder.WriteString("\n}")
	return builder.String()
}

func (self *CCodeGen) CodeGenReturnStatement(statement *ReturnStatement) string {
	return "return " + self.CodeGenExpression(statement.expression) + ";"
}

func (self *CCodeGen) CodeGenIfStatement(statement *IfStatement) string {
	builder := strings.Builder{}
	builder.WriteString("if ")
	builder.WriteString(self.CodeGenExpression(statement.condition))
	builder.WriteString(self.CodeGenScopeStatement(statement.scope))
	return builder.String()
}

func (self *CCodeGen) CodeGenFnStatement(statement *FnStatement) string {
	builder := strings.Builder{}
	builder.WriteString("void ")
	// TODO fix this
	//builder.WriteString(statement.identifier)
	builder.WriteString("()")
	builder.WriteString(self.CodeGenScopeStatement(statement.body))
	return builder.String()
}

func (self *CCodeGen) CodeGenExpression(expression Expression) string {
	switch expression.(type) {
	case *BoolLiteralExpression:
		return self.CodeGenBoolLiteral(expression.(*BoolLiteralExpression))
	case *IdentifierExpression:
		return self.CodeGenIdentifier(expression.(*IdentifierExpression))
	case *BinaryExpression:
		return self.CodeGenBinary(expression.(*BinaryExpression))
	case *GroupExpression:
		return self.CodeGenGroup(expression.(*GroupExpression))
	case *NumberLiteralExpression:
		return self.CodeGenNumberLiteral(expression.(*NumberLiteralExpression))
	default:
		panic("cannot code gen this expression, not implemented!!")
	}
}

func (self *CCodeGen) CodeGenBoolLiteral(expression *BoolLiteralExpression) string {
	// TODO make a way to get the string from the source with the token
	//if expression.value {
	return "true"
	//} else {
	//	return "false"
	//}
}

func (self *CCodeGen) CodeGenIdentifier(expression *IdentifierExpression) string {
	// TODO make a way to get the string from the source with the token
	//return expression.identifier
	return ""
}

func (self *CCodeGen) CodeGenBinary(expression *BinaryExpression) string {
	panic("not implemented")
}

func (self *CCodeGen) CodeGenGroup(expression *GroupExpression) string {
	builder := strings.Builder{}
	builder.WriteString("(")
	builder.WriteString(self.CodeGenExpression(expression.expression))
	builder.WriteString(")")
	return builder.String()
}

func (self *CCodeGen) CodeGenNumberLiteral(expression *NumberLiteralExpression) string {
	return strconv.FormatInt(int64(expression.number), 10)
}
