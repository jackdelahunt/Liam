package compiler

import (
	"testing"
)

const (
	ExampleTestCode = `
struct Person {

}

fn add() int {
	return x;
}

fn is_true() bool {
	return true;
}

fn main() int {
	return 0 + 5;
}
`
)

// used to just lex the source and get the tokens
func lexSource(source string) []TokenData {
	return NewLexer([]rune(source)).Lex()
}

// used to create a parse from the source and then can
// choose how to parse it
func createParserFromSource(source string) *Parser {
	tokens := NewLexer([]rune(source)).Lex()
	return NewParser([]rune(source), tokens)
}

// Used for full lexing and compiling on the source
func createASTFromSource(source string) (AST, error) {
	tokens := NewLexer([]rune(source)).Lex()
	return NewParser([]rune(source), tokens).Parse()
}

// Used for full lexing and compiling on the source
func createTypedASTFromSource(source string) (TypedAST, error) {
	tokens := NewLexer([]rune(source)).Lex()
	ast, err := NewParser([]rune(source), tokens).Parse()
	if err != nil {
		return TypedAST{}, err
	}

	return NewTypeChecker(ast).TypeCheck()
}

func Test_TypeChecker_EmptyMain(t *testing.T) {
	typedAST, err := createTypedASTFromSource("fn main() int {  }")
	if err != nil {
		t.Errorf(err.Error())
	}

	_ = typedAST
}

func Test_TypeChecker_Big(t *testing.T) {
	typedAST, err := createTypedASTFromSource(ExampleTestCode)
	if err != nil {
		t.Errorf(err.Error())
	}

	_ = typedAST
}

func Test_Parser_Big(t *testing.T) {
	_, err := createASTFromSource(ExampleTestCode)
	if err != nil {
		t.Errorf(err.Error())
	}
}

func Test_Lexer_Big(t *testing.T) {
	// TODO make the lexer generate errors
	_ = lexSource(ExampleTestCode)
}

func Test_Lexer_BinaryOperators(t *testing.T) {
	tokens := lexSource("+-*/")

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
	tokens := lexSource("   \n \t \r   ")
	if len(tokens) != 0 {
		t.Errorf("TokenBuffer should be empty %v", len(tokens))
	}
}

func Test_Lexer_Identifiers(t *testing.T) {
	tokens := lexSource("jack liam")

	if len(tokens) != 2 {
		t.Errorf("Expected 2 tokens got %v", len(tokens))
	}
}

func Test_Lexer_Numbers(t *testing.T) {
	tokens := lexSource("5 4 2")

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
	parser := createParserFromSource("return 2;")
	returnStatement, err := parser.ParseReturnStatement()
	if err != nil {
		t.Errorf(err.Error())
	}

	numberLiteral, ok := returnStatement.expression.(*NumberLiteralExpression)
	if ok != true {
		t.Errorf("expected number literal expression")
	}

	numberString := string(GetTokenSLice(numberLiteral.number, parser.TokenBuffer, parser.source))
	if numberString != "2" {
		t.Errorf("exptected 2 got %v", numberString)
	}
}

func Test_Parser_Scope(t *testing.T) {
	parser := createParserFromSource("{ return 2; }")
	parseStatement, err := parser.ParseScopeStatement()
	if err != nil {
		t.Errorf(err.Error())
	}

	if len(parseStatement.statements) != 1 {
		t.Errorf("only expected one statement in the scope")
	}
}

func Test_Parser_Scope_2(t *testing.T) {
	parser := createParserFromSource("{ return 2; return 3; }")
	parseStatement, err := parser.ParseScopeStatement()
	if err != nil {
		t.Errorf(err.Error())
	}

	if len(parseStatement.statements) != 2 {
		t.Errorf("expected two statement in the scope")
	}
}

func Test_Parser_Scope_Empty(t *testing.T) {
	parser := createParserFromSource("{}")
	parseStatement, err := parser.ParseScopeStatement()
	if err != nil {
		t.Errorf(err.Error())
	}

	if len(parseStatement.statements) != 0 {
		t.Errorf("expected no statement in the scope")
	}
}

func Test_Parser_Fn(t *testing.T) {
	parser := createParserFromSource("fn main() type { }")
	fnStatement, err := parser.ParseFnStatement()
	if err != nil {
		t.Errorf(err.Error())
	}

	_ = fnStatement
}

func Test_Parser_Binary(t *testing.T) {
	parser := createParserFromSource("1 + 2")
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

	lhsNumberString := GetTokenSLice(lhs.number, parser.TokenBuffer, parser.source)
	if string(lhsNumberString) != "1" {
		t.Errorf("expected 1 but got %v", lhsNumberString)
	}

	rhs, ok := binary.rhs.(*NumberLiteralExpression)
	if ok != true {
		t.Errorf("expected number literal expression")
	}

	rhsNumberString := GetTokenSLice(rhs.number, parser.TokenBuffer, parser.source)
	if string(rhsNumberString) != "2" {
		t.Errorf("expected 2 but got %v", rhsNumberString)
	}
}

func Test_Parser_NumberLiteral(t *testing.T) {
	parser := createParserFromSource("1")
	expression, err := parser.ParseExpression()
	if err != nil {
		t.Errorf(err.Error())
	}

	numberLiteralExpression, ok := expression.(*NumberLiteralExpression)
	if ok != true {
		t.Errorf("expected expression to be number literal")
	}

	numberString := GetTokenSLice(numberLiteralExpression.number, parser.TokenBuffer, parser.source)
	if string(numberString) != "1" {
		t.Errorf("expected number to be 1 got %v", numberString)
	}
}

func Test_Parser_Call(t *testing.T) {
	parser := createParserFromSource("add()")
	_, err := parser.ParseExpression()
	if err != nil {
		t.Errorf(err.Error())
	}
}
