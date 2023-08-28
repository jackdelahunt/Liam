package main

import (
	"testing"
)

func LexStringAndGetTokens(inputText string) []Token {
	return NewLexer().Lex(inputText)
}

func LexAndParseStringAndGetAST(inputText string) ([]Statement, error) {
	tokens := NewLexer().Lex(inputText)
	return NewParser(&tokens).Parse()
}

func LexAndMakeParse(inputText string) *Parser {
	tokens := NewLexer().Lex(inputText)
	return NewParser(&tokens)
}

func Test_Lexer_BinaryOperators(t *testing.T) {
	tokens := LexStringAndGetTokens("+-*/")

	if len(tokens) != 4 {
		t.Errorf("Need 4 tokens got %v", len(tokens))
	}

	if tokens[0].TokenType != Plus {
		t.Errorf("Expected + got %v", tokens[0].TokenType)
	}

	if tokens[1].TokenType != Minus {
		t.Errorf("Expected - got %v", tokens[0].TokenType)
	}

	if tokens[2].TokenType != Star {
		t.Errorf("Expected * got %v", tokens[0].TokenType)
	}

	if tokens[3].TokenType != Divide {
		t.Errorf("Expected / got %v", tokens[0].TokenType)
	}
}

func Test_Lexer_Whitespace(t *testing.T) {
	tokens := LexStringAndGetTokens("   \n \t \r   ")
	if len(tokens) != 0 {
		t.Errorf("Tokens should be empty %v", len(tokens))
	}
}

func Test_Lexer_Identifiers(t *testing.T) {
	tokens := LexStringAndGetTokens("jack liam")

	if len(tokens) != 2 {
		t.Errorf("Expected 2 tokens got %v", len(tokens))
	}
}

func Test_Lexer_Numbers(t *testing.T) {
	tokens := LexStringAndGetTokens("5 4 2")

	if len(tokens) != 3 {
		t.Errorf("Expected 3 tokens got %v", len(tokens))
	}

	if tokens[0].TokenType != NumberLiteral {
		t.Errorf("Expected number literal got %v", tokens[0].TokenType)
	}

	if tokens[1].TokenType != NumberLiteral {
		t.Errorf("Expected number literal got %v", tokens[1].TokenType)
	}

	if tokens[2].TokenType != NumberLiteral {
		t.Errorf("Expected number literal got %v", tokens[2].TokenType)
	}
}

func Test_Parser_Return(t *testing.T) {
	parser := LexAndMakeParse("return 2;")
	returnStatement, err := parser.ParseReturnStatement()
	if err != nil {
		t.Errorf(err.Error())
	}

	numberLiteral, ok := returnStatement.expression.(*NumberLiteralExpression)
	if ok != true {
		t.Errorf("expected number literal expression")
	}

	if numberLiteral.number != 2 {
		t.Errorf("exptected 2 got %v", numberLiteral.number)
	}
}

func Test_Parser_Scope(t *testing.T) {
	parser := LexAndMakeParse("{ return 2; }")
	parseStatement, err := parser.ParseScopeStatement()
	if err != nil {
		t.Errorf(err.Error())
	}

	if len(parseStatement.statements) != 1 {
		t.Errorf("only expected one statement in the scope")
	}
}

func Test_Parser_Scope_2(t *testing.T) {
	parser := LexAndMakeParse("{ return 2; return 3; }")
	parseStatement, err := parser.ParseScopeStatement()
	if err != nil {
		t.Errorf(err.Error())
	}

	if len(parseStatement.statements) != 2 {
		t.Errorf("expected two statement in the scope")
	}
}

func Test_Parser_Scope_Empty(t *testing.T) {
	parser := LexAndMakeParse("{}")
	parseStatement, err := parser.ParseScopeStatement()
	if err != nil {
		t.Errorf(err.Error())
	}

	if len(parseStatement.statements) != 0 {
		t.Errorf("expected no statement in the scope")
	}
}

func Test_Parser_Binary(t *testing.T) {
	parser := LexAndMakeParse("1 + 2")
	expression, err := parser.ParseExpression()
	if err != nil {
		t.Errorf(err.Error())
	}

	binary, ok := expression.(*BinaryExpression)
	if ok != true {
		t.Errorf("expected binary expression")
	}

	lhs, ok := binary.lhs.(*NumberLiteralExpression)
	if ok != true {
		t.Errorf("expected number literal expression")
	}

	if lhs.number != 1 {
		t.Errorf("expected 1 but got %v", lhs.number)
	}

	rhs, ok := binary.rhs.(*NumberLiteralExpression)
	if ok != true {
		t.Errorf("expected number literal expression")
	}

	if rhs.number != 2 {
		t.Errorf("expected 2 but got %v", rhs.number)
	}
}

func Test_Parser_Groups(t *testing.T) {
	parser := LexAndMakeParse("(5)")
	expression, err := parser.ParseExpression()
	if err != nil {
		t.Errorf(err.Error())
	}

	_, ok := expression.(*GroupExpression)
	if ok != true {
		t.Errorf("expected expression to be number literal")
	}
}

func Test_Parser_NumberLiteral(t *testing.T) {
	parser := LexAndMakeParse("1")
	expression, err := parser.ParseExpression()
	if err != nil {
		t.Errorf(err.Error())
	}

	numberLiteralExpression, ok := expression.(*NumberLiteralExpression)
	if ok != true {
		t.Errorf("expected expression to be number literal")
	}

	if numberLiteralExpression.number != 1 {
		t.Errorf("expected number to be 1 got %v", numberLiteralExpression.number)
	}
}
