package main

import "strings"

type TokenType = uint8

const (
	NumberLiteral TokenType = iota
	StringLiteral
	Let
	Fn
	Return
	ParenOpen
	ParenClose
	BraceOpen
	BraceClose
	Identifier
	BinaryOperator
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
		case '(':
			tokens = append(tokens, NewToken(ParenOpen, "("))
		case ')':
			tokens = append(tokens, NewToken(ParenClose, ")"))
		case '{':
			tokens = append(tokens, NewToken(BraceOpen, "{"))
		case '}':
			tokens = append(tokens, NewToken(BraceClose, "}"))
		case '+':
			tokens = append(tokens, NewToken(BinaryOperator, "+"))
		case '-':
			tokens = append(tokens, NewToken(BinaryOperator, "-"))
		case '*':
			tokens = append(tokens, NewToken(BinaryOperator, "*"))
		case '/':
			tokens = append(tokens, NewToken(BinaryOperator, "/"))
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

type Statement interface{}

type ExpressionStatement struct {
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

type NumberLiteralExpression struct {
	number int
}

func (self *NumberLiteralExpression) TypeInfo() *TypeInfo {
	return nil
}

func main() {
}
