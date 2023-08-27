package main

import (
	"testing"
)

func LexStringAndGetTokens(inputText string) []Token {
	return NewLexer().Lex(inputText)
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

func Test_Parser_Tree(t *testing.T) {
	//statement := ExpressionStatement{
	//	expression: &BinaryExpression{
	//		lhs:      &NumberLiteralExpression{number: 5},
	//		operator: NewToken(BinaryOperator, "+"),
	//		rhs:      &NumberLiteralExpression{number: 5},
	//	},
	//}
}
