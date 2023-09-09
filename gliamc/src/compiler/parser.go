package compiler

import (
	"errors"
	"fmt"
	"log"
	"strings"
)

type OperatorType = uint

const (
	OPAdd OperatorType = iota
	OPMinus
	OPMultiply
	OPDivide
)

type AST struct {
	Source      []rune
	TokenBuffer []TokenData
	Statements  []Statement
}

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

/*
	   ============
		Statements
	   ============
*/
type Statement interface {
}

type ScopeStatement struct {
	statements  []Statement
	symbolTable LocalSymbolTable
}

type ReturnStatement struct {
	expression Expression
}

type FnStatement struct {
	Identifier  Token
	ReturnType  TypeExpression
	Body        []Statement
	SymbolTable *LocalSymbolTable
}
type StructStatement struct {
	Identifier Token
}

/*
	   ============
		Expressions
	   ============
*/
type Expression interface {
	TypeInfo() TypeInfo
}

type BoolLiteralExpression struct {
	value    Token
	typeInfo TypeInfo
}

func (self *BoolLiteralExpression) TypeInfo() TypeInfo {
	return self.typeInfo
}

type IdentifierExpression struct {
	identifier Token
	typeInfo   TypeInfo
}

func (self *IdentifierExpression) TypeInfo() TypeInfo {
	return self.typeInfo
}

type BinaryExpression struct {
	lhs          Expression
	operator     Token
	rhs          Expression
	operatorType OperatorType
	typeInfo     TypeInfo
}

func (self *BinaryExpression) TypeInfo() TypeInfo {
	return self.typeInfo
}

type CallExpression struct {
	callee   Expression
	typeInfo TypeInfo
}

func (self *CallExpression) TypeInfo() TypeInfo {
	return self.typeInfo
}

type NumberLiteralExpression struct {
	number   Token
	typeInfo TypeInfo
}

func (self *NumberLiteralExpression) TypeInfo() TypeInfo {
	return self.typeInfo
}

/*
	   ============
		Type Expressions
	   ============
*/
type TypeExpression interface {
	TypeInfo() TypeInfo
}

type IdentifierTypeExpression struct {
	identifier Token
	typeInfo   TypeInfo
}

func (self *IdentifierTypeExpression) TypeInfo() TypeInfo {
	return self.typeInfo
}

/*
	   ============
		Parsing
	   ============
*/
type Parser struct {
	source       []rune
	TokenBuffer  []TokenData
	CurrentToken Token
}

func NewParser(source []rune, tokens []TokenData) *Parser {
	return &Parser{
		source:       source,
		TokenBuffer:  tokens,
		CurrentToken: 0,
	}
}

func (self *Parser) Parse() (AST, error) {
	statements := make([]Statement, 0)

	for self.CurrentToken < Token(len(self.TokenBuffer)) {
		statement, err := self.ParseStatement()
		if err != nil {
			return AST{}, err
		}

		statements = append(statements, statement)
	}

	return AST{
		Source:      self.source,
		TokenBuffer: self.TokenBuffer,
		Statements:  statements,
	}, nil
}

func (self *Parser) ParseStatement() (Statement, error) {
	currentToken := self.Peek()

	switch self.TokenBuffer[currentToken].TokenType {
	case Return:
		return self.ParseReturnStatement()
	case BraceOpen:
		return self.ParseScopeStatement()
	case Fn:
		return self.ParseFnStatement()
	case Struct:
		return self.ParseStructStatement()
	}

	return nil, errors.New("unexpected token at the start of a statement")
}

func (self *Parser) ParseReturnStatement() (*ReturnStatement, error) {
	_, err := self.ConsumeTokenOfType(Return)
	if err != nil {
		return nil, err
	}

	expression, err := self.ParseExpression()
	if err != nil {
		return nil, err
	}

	_, err = self.ConsumeTokenOfType(SemiColon)
	if err != nil {
		return nil, err
	}

	return &ReturnStatement{expression: expression}, nil
}

func (self *Parser) ParseScopeStatement() (*ScopeStatement, error) {
	statements := make([]Statement, 0)

	_, err := self.ConsumeTokenOfType(BraceOpen)
	if err != nil {
		return nil, err
	}

	closingIndex, err := self.FindBalancePoint(BraceOpen, BraceClose, self.CurrentToken-1)
	if err != nil {
		return nil, err
	}

	for self.CurrentToken < closingIndex {
		statement, err := self.ParseStatement()
		if err != nil {
			return nil, err
		}

		statements = append(statements, statement)
	}

	_, err = self.ConsumeTokenOfType(BraceClose)
	if err != nil {
		return nil, err
	}

	return &ScopeStatement{statements: statements}, nil
}

func (self *Parser) ParseFnStatement() (*FnStatement, error) {
	_, err := self.ConsumeTokenOfType(Fn)
	if err != nil {
		return nil, err
	}

	identifier, err := self.ConsumeTokenOfType(Identifier)
	if err != nil {
		return nil, err
	}

	_, err = self.ConsumeTokenOfType(ParenOpen)
	if err != nil {
		return nil, err
	}

	_, err = self.ConsumeTokenOfType(ParenClose)
	if err != nil {
		return nil, err
	}

	typeExpression, err := self.ParseTypeExpression()
	if err != nil {
		return nil, err
	}

	body, err := self.ParseStatementsInBrace()
	if err != nil {
		return nil, err
	}

	return &FnStatement{Identifier: identifier, ReturnType: typeExpression, Body: body}, nil
}

func (self *Parser) ParseStructStatement() (*StructStatement, error) {
	_, err := self.ConsumeTokenOfType(Struct)
	if err != nil {
		return nil, err
	}

	identifier, err := self.ConsumeTokenOfType(Identifier)
	if err != nil {
		return nil, err
	}

	_, err = self.ConsumeTokenOfType(BraceOpen)
	if err != nil {
		return nil, err
	}

	_, err = self.ConsumeTokenOfType(BraceClose)
	if err != nil {
		return nil, err
	}

	return &StructStatement{Identifier: identifier}, nil
}

func (self *Parser) ParseStatementsInBrace() ([]Statement, error) {
	statements := make([]Statement, 0)

	_, err := self.ConsumeTokenOfType(BraceOpen)
	if err != nil {
		return nil, err
	}

	closingIndex, err := self.FindBalancePoint(BraceOpen, BraceClose, self.CurrentToken-1)
	if err != nil {
		return nil, err
	}

	for self.CurrentToken < closingIndex {
		statement, err := self.ParseStatement()
		if err != nil {
			return nil, err
		}

		statements = append(statements, statement)
	}

	_, err = self.ConsumeTokenOfType(BraceClose)
	if err != nil {
		return nil, err
	}

	return statements, nil
}

/*
=== expression precedence === (lower is more precedence)
or
and
== !=
< > >= <=
+ -
?
* / %
@ * !
call() slice[0]
literal () new "" null true false []u64{}
*/
func (self *Parser) ParseExpression() (Expression, error) {
	return self.ParseTerm()
}

func (self *Parser) ParseTerm() (Expression, error) {
	expression, err := self.ParseFactor()
	if err != nil {
		return nil, err
	}

	for self.Match(Plus) || self.Match(Minus) {
		operator, err := self.ConsumeToken()
		if err != nil {
			return nil, err
		}

		rightExpression, err := self.ParseFactor()
		if err != nil {
			return nil, err
		}

		expression = &BinaryExpression{lhs: expression, operator: operator, rhs: rightExpression}
	}

	return expression, err
}

func (self *Parser) ParseFactor() (Expression, error) {
	expression, err := self.ParsePostfix()
	if err != nil {
		return nil, err
	}

	for self.Match(Star) || self.Match(Divide) {
		operator, err := self.ConsumeToken()
		if err != nil {
			return nil, err
		}

		rightExpression, err := self.ParsePostfix()
		if err != nil {
			return nil, err
		}

		expression = &BinaryExpression{lhs: expression, operator: operator, rhs: rightExpression}
	}

	return expression, err
}

func (self *Parser) ParsePostfix() (Expression, error) {
	expression, err := self.ParsePrimary()
	if err != nil {
		return nil, err
	}

	for {
		if self.Match(ParenOpen) {
			_, _ = self.ConsumeToken()
			_, err := self.ConsumeTokenOfType(ParenClose)
			if err != nil {
				return nil, err
			}

			expression = &CallExpression{callee: expression}
		} else {
			break
		}
	}

	return expression, nil
}

/*
Expression *Parser::eval_postfix() {
    auto expr = TRY_CALL_RET(eval_primary(), NULL);

    while (true)
    {
        if (match(TokenType::TOKEN_PAREN_OPEN) || match(TokenType::TOKEN_COLON))
        {
            TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_PAREN_OPEN), NULL);
            auto call_args = TRY_CALL_RET(consume_comma_seperated_arguments(TokenType::TOKEN_PAREN_CLOSE), NULL);
            TRY_CALL_RET(consume_token_of_type(TokenType::TOKEN_PAREN_CLOSE), NULL);

            expr = new CallExpression(expr, call_args, generics);
        }
        else
        { break; }
    }

    return expr;
}
*/

func (self *Parser) ParsePrimary() (Expression, error) {
	token := self.Peek()
	currentTokenType := self.TokenBuffer[token].TokenType

	switch currentTokenType {
	case NumberLiteral:
		return self.ParseNumberLiteral()
	case Identifier:
		return self.ParseIdentifier()
	case TrueLiteral, FalseLiteral:
		return self.ParseBoolLiteral()
	default:
		return nil, fmt.Errorf("unknown token when parsing primary %v", currentTokenType)
	}
}

func (self *Parser) ParseBoolLiteral() (Expression, error) {
	value, err := self.ConsumeToken()
	if err != nil {
		return nil, err
	}

	return &BoolLiteralExpression{value: value}, nil
}

func (self *Parser) ParseIdentifier() (Expression, error) {
	identifier, err := self.ConsumeTokenOfType(Identifier)
	if err != nil {
		return nil, err
	}

	return &IdentifierExpression{identifier: identifier}, nil
}

func (self *Parser) ParseNumberLiteral() (Expression, error) {
	number, err := self.ConsumeTokenOfType(NumberLiteral)
	if err != nil {
		return nil, err
	}

	return &NumberLiteralExpression{number: number}, nil
}

/*
	   ============
		Type Expressions
	   ============
*/
func (self *Parser) ParseTypeExpression() (TypeExpression, error) {
	return self.ParsePrimaryTypeExpression()
}

func (self *Parser) ParsePrimaryTypeExpression() (TypeExpression, error) {
	identifier, err := self.ConsumeTokenOfType(Identifier)
	if err != nil {
		return nil, err
	}

	return &IdentifierTypeExpression{identifier: identifier}, nil
}

func (self *Parser) Peek() Token {
	return self.CurrentToken
}

func (self *Parser) Match(tokenType TokenType) bool {
	if self.CurrentToken >= Token(len(self.TokenBuffer)) {
		return false
	}
	return self.TokenBuffer[self.CurrentToken].TokenType == tokenType
}

func (self *Parser) ConsumeToken() (Token, error) {
	token := self.CurrentToken
	if token >= Token(len(self.TokenBuffer)) {
		return 0, fmt.Errorf("unexpected end of file")
	}

	self.CurrentToken += 1
	return token, nil
}

func (self *Parser) ConsumeTokenOfType(tokenType TokenType) (Token, error) {
	token := self.CurrentToken
	currentTokenType := self.TokenBuffer[token].TokenType
	if currentTokenType != tokenType {
		return 0, fmt.Errorf("unexpected token, expected type %v got %v", tokenType, currentTokenType)
	}

	self.CurrentToken += 1
	return token, nil
}

func GetTokenSLice(token Token, tokenBuffer []TokenData, source []rune) []rune {
	location := tokenBuffer[token].Location

	if location.Start >= uint(len(source)) || location.End >= uint(len(source)) {
		log.Fatal(fmt.Sprintf("getting slice from source with unbounded range :: start %v : end %v : source length %v", location.Start, location.End, len(source)))
	}

	return source[location.Start : location.End+1]
}

func (self *Parser) FindBalancePoint(push TokenType, pull TokenType, startingPoint Token) (Token, error) {
	currentIndex := startingPoint
	balanceCount := int(0)

	for self.CurrentToken < Token(len(self.TokenBuffer)) {
		if self.TokenBuffer[currentIndex].TokenType == push {
			balanceCount += 1
			if balanceCount == 0 {
				return currentIndex, nil
			}
		}

		if self.TokenBuffer[currentIndex].TokenType == pull {
			balanceCount -= 1
			if balanceCount == 0 {
				return currentIndex, nil
			}
		}

		currentIndex += 1
	}

	return currentIndex, nil
}
