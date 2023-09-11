package compiler

import (
	"fmt"
	"log"
	"strings"
)

func GetIndentationString(indentation int) string {
	builder := strings.Builder{}
	for i := 0; i < indentation; i += 1 {
		builder.WriteString("  ")
	}

	return builder.String()
}

func (self *AST) GetTokenSlice(token Token) []rune {
	return GetTokenSLice(token, self.TokenBuffer, self.Source)
}

func (self *AST) PrintAST() {
	for _, statement := range self.Statements {
		fmt.Println(self.StatementToString(0, statement))
	}
}

func (self *AST) StatementToString(indentation int, statement Statement) string {
	switch statement.(type) {
	case *ScopeStatement:
		return self.ScopeStatementToString(indentation, statement.(*ScopeStatement))
	case *ReturnStatement:
		return self.ReturnStatementToString(indentation, statement.(*ReturnStatement))
	case *FnStatement:
		return self.FnStatementToString(indentation, statement.(*FnStatement))
	case *StructStatement:
		return self.StructStatementToString(indentation, statement.(*StructStatement))

	default:
		log.Fatal("cannot to string this statement when printing in the AST")
	}

	return ""
}

func (self *AST) ScopeStatementToString(indentation int, statement *ScopeStatement) string {
	builder := strings.Builder{}
	builder.WriteString(fmt.Sprintf("scope\n"))
	for _, subStatement := range statement.statements {
		builder.WriteString(fmt.Sprintf("%v-%v\n", GetIndentationString(indentation+1), self.StatementToString(indentation+1, subStatement)))
	}

	return builder.String()
}

func (self *AST) ReturnStatementToString(indentation int, statement *ReturnStatement) string {
	builder := strings.Builder{}
	builder.WriteString(fmt.Sprintf("<return>\n"))
	builder.WriteString(fmt.Sprintf("%v- %v", GetIndentationString(indentation+1), self.ExpressionToString(indentation+1, statement.expression)))
	return builder.String()
}

func (self *AST) FnStatementToString(indentation int, statement *FnStatement) string {
	builder := strings.Builder{}
	builder.WriteString(fmt.Sprintf("<fn> %v\n", string(self.GetTokenSlice(statement.Identifier))))

	// return expression
	builder.WriteString(fmt.Sprintf("%v| return type\n", GetIndentationString(indentation+1)))
	builder.WriteString(fmt.Sprintf("%v- %v\n", GetIndentationString(indentation+2), self.TypeExpressionToString(indentation+2, statement.ReturnType)))

	// body area
	builder.WriteString(fmt.Sprintf("%v| body\n", GetIndentationString(indentation+1)))
	for _, subStatement := range statement.Body {
		builder.WriteString(fmt.Sprintf("%v- %v\n", GetIndentationString(indentation+2), self.StatementToString(indentation+2, subStatement)))
	}

	return builder.String()
}

func (self *AST) StructStatementToString(indentation int, statement *StructStatement) string {
	return fmt.Sprintf("<struct> %v\n", string(self.GetTokenSlice(statement.Identifier)))
}

func (self *AST) ExpressionToString(indentation int, expression Expression) string {
	switch expression.(type) {
	case *BoolLiteralExpression:
		return self.BoolLiteralExpressionToString(expression.(*BoolLiteralExpression))
	case *IdentifierExpression:
		return self.IdentifierExpressionToString(expression.(*IdentifierExpression))
	case *BinaryExpression:
		return self.BinaryExpressionToString(indentation, expression.(*BinaryExpression))
	case *CallExpression:
		return self.CallExpressionToString(indentation, expression.(*CallExpression))
	case *NumberLiteralExpression:
		return self.NumberLiteralExpressionToString(expression.(*NumberLiteralExpression))

	default:
		log.Fatal("cannot to string this statement when printing in the AST")
	}

	return ""
}

func (self *AST) BoolLiteralExpressionToString(expression *BoolLiteralExpression) string {
	if self.TokenBuffer[expression.value].TokenType == TrueLiteral {
		return "<bool> true"
	} else {
		return "<bool> false"
	}
}

func (self *AST) IdentifierExpressionToString(expression *IdentifierExpression) string {
	return "<identifier> " + string(self.GetTokenSlice(expression.identifier))
}

func (self *AST) BinaryExpressionToString(indentation int, expression *BinaryExpression) string {
	builder := strings.Builder{}
	builder.WriteString("<binary> ")
	builder.WriteString(string(self.GetTokenSlice(expression.operator)) + "\n")
	builder.WriteString(fmt.Sprintf("%v- %v\n", GetIndentationString(indentation+1), self.ExpressionToString(indentation+1, expression.lhs)))
	builder.WriteString(fmt.Sprintf("%v- %v", GetIndentationString(indentation+1), self.ExpressionToString(indentation+1, expression.rhs)))
	return builder.String()
}

func (self *AST) CallExpressionToString(indentation int, expression *CallExpression) string {
	builder := strings.Builder{}
	builder.WriteString("<call>\n")
	builder.WriteString(fmt.Sprintf("%v- %v", GetIndentationString(indentation+1), self.ExpressionToString(indentation+1, expression.callee)))
	return builder.String()
}

func (self *AST) NumberLiteralExpressionToString(expression *NumberLiteralExpression) string {
	return "<int> " + string(self.GetTokenSlice(expression.number))
}

func (self *AST) TypeExpressionToString(indentation int, expression TypeExpression) string {
	switch expression.(type) {
	case *IdentifierTypeExpression:
		return self.IdentifierTypeExpression(expression.(*IdentifierTypeExpression))

	default:
		log.Fatal("cannot to string this type expression when printing in the AST")
	}

	return ""
}

func (self *AST) IdentifierTypeExpression(expression *IdentifierTypeExpression) string {
	return "<identifier> " + string(self.GetTokenSlice(expression.identifier))
}
