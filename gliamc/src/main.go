package main

import (
	"errors"
	"fmt"
	"strconv"
	"strings"
)

type TokenType = uint8

const (
	NumberLiteral TokenType = iota
	StringLiteral
	SemiColon
	Let
	Fn
	Return
	ParenOpen
	ParenClose
	BraceOpen
	BraceClose
	Identifier
	Plus
	Minus
	Star
	Divide
)

type Token struct {
	TokenType TokenType
	Text      string
}

func NewToken(tokenType TokenType, text string) Token {
	return Token{
		tokenType,
		text,
	}
}

type Lexer struct {
	CurrentIndex     uint
	CurrentLine      uint
	CurrentCharacter uint
	Text             []rune
}

func NewLexer() *Lexer {
	return &Lexer{
		CurrentIndex:     0,
		CurrentLine:      0,
		CurrentCharacter: 0,
		Text:             nil,
	}
}

func (self *Lexer) Lex(input string) []Token {
	self.Text = []rune(input)
	tokens := make([]Token, 0)

	for self.CurrentIndex < uint(len(self.Text)) {
		character := self.Text[self.CurrentIndex]
		switch character {
		// FIXME this is temp
		case '0', '1', '2', '3', '4', '5', '6', '7', '8', '9':
			tokens = append(tokens, NewToken(NumberLiteral, string(character)))
		case ' ', '\t', '\r', '\n':
		case ';':
			tokens = append(tokens, NewToken(SemiColon, ";"))
		case '(':
			tokens = append(tokens, NewToken(ParenOpen, "("))
		case ')':
			tokens = append(tokens, NewToken(ParenClose, ")"))
		case '{':
			tokens = append(tokens, NewToken(BraceOpen, "{"))
		case '}':
			tokens = append(tokens, NewToken(BraceClose, "}"))
		case '+':
			tokens = append(tokens, NewToken(Plus, "+"))
		case '-':
			tokens = append(tokens, NewToken(Minus, "-"))
		case '*':
			tokens = append(tokens, NewToken(Star, "*"))
		case '/':
			tokens = append(tokens, NewToken(Divide, "/"))
		default:
			word := self.GetWord()

			switch word {
			case "let":
				tokens = append(tokens, NewToken(Let, word))
			case "fn":
				tokens = append(tokens, NewToken(Fn, word))
			case "return":
				tokens = append(tokens, NewToken(Return, word))
			default:
				tokens = append(tokens, NewToken(Identifier, word))
			}
		}

		self.CurrentIndex += 1
	}

	return tokens
}

func (self *Lexer) GetWord() string {
	word := strings.Builder{}

	for self.CurrentIndex < uint(len(self.Text)) && !IsDelimeter(self.Text[self.CurrentIndex]) {
		character := self.Text[self.CurrentIndex]
		word.WriteRune(character)
		self.Next()
	}

	// removing the last Next call because it will mean the
	// current index points to the rune after the word
	// but it then is skipped as we iterate the current
	// index at the end of each loop in the Lex func
	self.Back()

	return word.String()
}

func (self *Lexer) Next() {
	self.CurrentIndex += 1

	// TODO turn these on at some point
	// self.current_character += 1
	// self.current_line += 1
}

func (self *Lexer) Back() {
	self.CurrentIndex -= 1

	// TODO turn this on at some point
	// self.current_character -= 1
}

func IsDelimeter(c rune) bool {
	return c == ' ' || c == '\n' || c == ';' || c == '(' || c == ')' || c == '{' || c == '}' || c == ',' || c == ':' ||
		c == '=' || c == '+' || c == '^' || c == '&' || c == '*' || c == '.' || c == '[' || c == ']' || c == '!' ||
		c == '<' || c == '>' || c == '|' || c == '-' || c == '/' || c == '%'
}

type TypeInfo struct{}

type Statement interface {
}

type ExpressionStatement struct {
	expression Expression
}

type ReturnStatement struct {
	expression Expression
}

type Expression interface {
	TypeInfo() *TypeInfo
}

type BinaryExpression struct {
	lhs      Expression
	operator Token
	rhs      Expression
}

func (self *BinaryExpression) TypeInfo() *TypeInfo {
	return nil
}

type GroupExpression struct {
	expression Expression
}

func (self *GroupExpression) TypeInfo() *TypeInfo {
	return nil
}

type NumberLiteralExpression struct {
	number int
}

func (self *NumberLiteralExpression) TypeInfo() *TypeInfo {
	return nil
}

type Parser struct {
	CurrentTokenIndex uint
	Tokens            *[]Token
}

func NewParser(tokens *[]Token) *Parser {
	return &Parser{
		CurrentTokenIndex: 0,
		Tokens:            tokens,
	}
}

func (self *Parser) Parse() ([]Statement, error) {
	topLevelStatements := make([]Statement, 0)

	for self.CurrentTokenIndex < uint(len(*self.Tokens)) {
		statement, err := self.ParseStatement()
		if err != nil {
			return nil, err
		}

		topLevelStatements = append(topLevelStatements, statement)
	}

	return topLevelStatements, nil
}

func (self *Parser) ParseStatement() (Statement, error) {
	currentToken := self.Peek()

	switch currentToken.TokenType {
	case Return:
		return self.ParseReturnStatement()
	}

	return nil, errors.New("unexpected token when parsing statements")
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
	expression, err := self.ParsePrimary()
	if err != nil {
		return nil, err
	}

	for self.Match(Plus) || self.Match(Minus) {
		token, err := self.ConsumeToken()
		if err != nil {
			return nil, err
		}

		rightExpression, err := self.ParsePrimary()
		if err != nil {
			return nil, err
		}

		expression = &BinaryExpression{lhs: expression, operator: *token, rhs: rightExpression}
	}

	return expression, err
}

func (self *Parser) ParsePrimary() (Expression, error) {
	token := self.Peek()

	switch token.TokenType {
	case ParenOpen:
		return self.ParseGroupExpression()
	case NumberLiteral:
		return self.ParseNumberLiteral()
	default:
		return nil, fmt.Errorf("unknown token %v", token.TokenType)
	}
}

func (self *Parser) ParseNumberLiteral() (Expression, error) {
	token, err := self.ConsumeTokenOfType(NumberLiteral)
	if err != nil {
		return nil, err
	}

	number, err := strconv.Atoi(token.Text)
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

func (self *Parser) Peek() *Token {
	return &(*self.Tokens)[self.CurrentTokenIndex]
}

func (self *Parser) Match(tokenType TokenType) bool {
	if self.CurrentTokenIndex >= uint(len(*self.Tokens)) {
		return false
	}
	return (*self.Tokens)[self.CurrentTokenIndex].TokenType == tokenType
}

func (self *Parser) ConsumeToken() (*Token, error) {
	token := &(*self.Tokens)[self.CurrentTokenIndex]
	self.CurrentTokenIndex += 1
	return token, nil
}

func (self *Parser) ConsumeTokenOfType(tokenType TokenType) (*Token, error) {
	token := &(*self.Tokens)[self.CurrentTokenIndex]
	if token.TokenType != tokenType {
		return nil, errors.New(fmt.Sprintf("Unexpected token, expected type %v got %v", tokenType, token.TokenType))
	}

	self.CurrentTokenIndex += 1

	return token, nil
}

func main() {
}
