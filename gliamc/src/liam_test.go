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

	// check the token type of each
	for _, token := range tokens {
		if token.TokenType != BinaryOperator {
			t.Errorf("Expected token type to be binary op %v", token.TokenType)
		}
	}

	if tokens[0].Text != "+" {
		t.Errorf("Expected + got %v", tokens[0].Text)
	}

	if tokens[1].Text != "-" {
		t.Errorf("Expected + got %v", tokens[1].Text)
	}

	if tokens[2].Text != "*" {
		t.Errorf("Expected + got %v", tokens[2].Text)
	}

	if tokens[3].Text != "/" {
		t.Errorf("Expected + got %v", tokens[3].Text)
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
