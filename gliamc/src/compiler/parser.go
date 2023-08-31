package compiler

import (
	"errors"
	"fmt"
	"log"
)

type AST struct {
	Source           []rune
	TokenBuffer      []TokenData
	FnStatements     []*FnStatement
	StructStatements []*StructStatement
}

/*
	   ============
		Statements
	   ============
*/
type Statement interface {
}

type ExpressionStatement struct {
	expression Expression
}

type ScopeStatement struct {
	statements []Statement
}

type ReturnStatement struct {
	expression Expression
}

type IfStatement struct {
	condition Expression
	scope     *ScopeStatement
}

type FnStatement struct {
	identifier Token
	returnType TypeExpression
	body       *ScopeStatement
}

type StructStatement struct {
	identifier Token
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
	lhs      Expression
	operator Token
	rhs      Expression
	typeInfo TypeInfo
}

func (self *BinaryExpression) TypeInfo() TypeInfo {
	return self.typeInfo
}

type GroupExpression struct {
	expression Expression
	typeInfo   TypeInfo
}

func (self *GroupExpression) TypeInfo() TypeInfo {
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
}

type IdentifierTypeExpression struct {
	identifier Token
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
	fnStatements := make([]*FnStatement, 0)
	structStatements := make([]*StructStatement, 0)

	for self.CurrentToken < Token(len(self.TokenBuffer)) {
		statement, err := self.ParseStatement()
		if err != nil {
			return AST{}, err
		}

		switch statement.(type) {
		case *FnStatement:
			fnStatements = append(fnStatements, statement.(*FnStatement))
		case *StructStatement:
			structStatements = append(structStatements, statement.(*StructStatement))
		default:
			log.Fatal("unknown statement at top level")
		}

	}

	return AST{
		Source:           self.source,
		TokenBuffer:      self.TokenBuffer,
		FnStatements:     fnStatements,
		StructStatements: structStatements,
	}, nil
}

func (self *Parser) ParseStatement() (Statement, error) {
	currentToken := self.Peek()

	switch self.TokenBuffer[currentToken].TokenType {
	case Return:
		return self.ParseReturnStatement()
	case BraceOpen:
		return self.ParseScopeStatement()
	case If:
		return self.ParseIfStatement()
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

func (self *Parser) ParseIfStatement() (*IfStatement, error) {
	_, err := self.ConsumeTokenOfType(If)
	if err != nil {
		return nil, err
	}

	condition, err := self.ParseExpression()
	if err != nil {
		return nil, err
	}

	scope, err := self.ParseScopeStatement()
	if err != nil {
		return nil, err
	}

	return &IfStatement{condition: condition, scope: scope}, nil
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

	body, err := self.ParseScopeStatement()
	if err != nil {
		return nil, err
	}

	// TODO make the identifier correct here
	return &FnStatement{identifier: identifier, returnType: typeExpression, body: body}, nil
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

	return &StructStatement{identifier: identifier}, nil
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
	expression, err := self.ParsePrimary()
	if err != nil {
		return nil, err
	}

	for self.Match(Star) || self.Match(Divide) {
		operator, err := self.ConsumeToken()
		if err != nil {
			return nil, err
		}

		rightExpression, err := self.ParsePrimary()
		if err != nil {
			return nil, err
		}

		expression = &BinaryExpression{lhs: expression, operator: operator, rhs: rightExpression}
	}

	return expression, err
}

func (self *Parser) ParsePrimary() (Expression, error) {
	token := self.Peek()
	currentTokenType := self.TokenBuffer[token].TokenType

	switch currentTokenType {
	case ParenOpen:
		return self.ParseGroupExpression()
	case NumberLiteral:
		return self.ParseNumberLiteral()
	case Identifier:
		return self.ParseIdentifier()
	case BoolLiteral:
		return self.ParseBoolLiteral()
	default:
		return nil, fmt.Errorf("unknown token when parsing primary %v", currentTokenType)
	}
}

func (self *Parser) ParseBoolLiteral() (Expression, error) {
	value, err := self.ConsumeTokenOfType(BoolLiteral)
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

func (self *Parser) ParseGroupExpression() (Expression, error) {
	_, err := self.ConsumeTokenOfType(ParenOpen)
	if err != nil {
		return nil, err
	}

	expression, err := self.ParseExpression()
	if err != nil {
		return nil, err
	}

	_, err = self.ConsumeTokenOfType(ParenClose)
	if err != nil {
		return nil, err
	}

	return &GroupExpression{expression: expression}, nil
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
