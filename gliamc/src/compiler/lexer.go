package compiler

type TokenType = uint8

const (
	NumberLiteral TokenType = iota
	StringLiteral
	BoolLiteral
	SemiColon
	Let
	Fn
	Struct
	If
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

type Token = uint

type TokenData struct {
	TokenType TokenType
	Location  Location
}

type Location struct {
	Start uint
	End   uint
}

func NewTokenData(tokenType TokenType, start uint, end uint) TokenData {
	return TokenData{
		TokenType: tokenType,
		Location: Location{
			Start: start, // TODO we do not use these yet
			End:   end,   // TODO we do not use these yet
		},
	}
}

func NewSingleTokenData(tokenType TokenType, start uint) TokenData {
	return NewTokenData(tokenType, start, start)
}

type Lexer struct {
	CurrentIndex uint
	Source       []rune
	TokenBuffer  []TokenData
}

func NewLexer(source []rune) *Lexer {
	return &Lexer{
		CurrentIndex: 0,
		Source:       source,
	}
}

func (self *Lexer) Lex() []TokenData {
	// TODO make this a static size based on the input
	self.TokenBuffer = make([]TokenData, 0)

	for self.CurrentIndex < uint(len(self.Source)) {
		character := self.Source[self.CurrentIndex]
		switch character {
		// FIXME this is temp
		case '0', '1', '2', '3', '4', '5', '6', '7', '8', '9':
			self.TokenBuffer = append(self.TokenBuffer, NewSingleTokenData(NumberLiteral, self.CurrentIndex))
		case ' ', '\t', '\r', '\n':
		case ';':
			self.TokenBuffer = append(self.TokenBuffer, NewSingleTokenData(SemiColon, self.CurrentIndex))
		case '(':
			self.TokenBuffer = append(self.TokenBuffer, NewSingleTokenData(ParenOpen, self.CurrentIndex))
		case ')':
			self.TokenBuffer = append(self.TokenBuffer, NewSingleTokenData(ParenClose, self.CurrentIndex))
		case '{':
			self.TokenBuffer = append(self.TokenBuffer, NewSingleTokenData(BraceOpen, self.CurrentIndex))
		case '}':
			self.TokenBuffer = append(self.TokenBuffer, NewSingleTokenData(BraceClose, self.CurrentIndex))
		case '+':
			self.TokenBuffer = append(self.TokenBuffer, NewSingleTokenData(Plus, self.CurrentIndex))
		case '-':
			self.TokenBuffer = append(self.TokenBuffer, NewSingleTokenData(Minus, self.CurrentIndex))
		case '*':
			self.TokenBuffer = append(self.TokenBuffer, NewSingleTokenData(Star, self.CurrentIndex))
		case '/':
			self.TokenBuffer = append(self.TokenBuffer, NewSingleTokenData(Divide, self.CurrentIndex))
		default:
			wordLocation := self.GetWord()

			switch string(GetSliceFromLocation(wordLocation, self.Source)) {
			case "let":
				self.TokenBuffer = append(self.TokenBuffer, NewTokenData(Let, wordLocation.Start, wordLocation.End))
			case "fn":
				self.TokenBuffer = append(self.TokenBuffer, NewTokenData(Fn, wordLocation.Start, wordLocation.End))
			case "struct":
				self.TokenBuffer = append(self.TokenBuffer, NewTokenData(Struct, wordLocation.Start, wordLocation.End))
			case "return":
				self.TokenBuffer = append(self.TokenBuffer, NewTokenData(Return, wordLocation.Start, wordLocation.End))
			case "if":
				self.TokenBuffer = append(self.TokenBuffer, NewTokenData(If, wordLocation.Start, wordLocation.End))
			case "true", "false":
				self.TokenBuffer = append(self.TokenBuffer, NewTokenData(BoolLiteral, wordLocation.Start, wordLocation.End))
			default:
				self.TokenBuffer = append(self.TokenBuffer, NewTokenData(Identifier, wordLocation.Start, wordLocation.End))
			}
		}

		self.CurrentIndex += 1
	}

	return self.TokenBuffer
}

func (self *Lexer) GetWord() Location {
	location := Location{
		Start: self.CurrentIndex,
		End:   0, // will be filled in at the end...
	}

	for self.CurrentIndex < uint(len(self.Source)) && !IsDelimeter(self.Source[self.CurrentIndex]) {
		self.Next()
	}

	// removing the last Next call because it will mean the
	// current index points to the rune after the word
	// but it then is skipped as we iterate the current
	// index at the end of each loop in the Lex func
	self.Back()

	location.End = self.CurrentIndex
	return location
}

func (self *Lexer) Next() {
	self.CurrentIndex += 1
}

func (self *Lexer) Back() {
	self.CurrentIndex -= 1
}

func IsDelimeter(c rune) bool {
	return c == ' ' || c == '\n' || c == ';' || c == '(' || c == ')' || c == '{' || c == '}' || c == ',' || c == ':' ||
		c == '=' || c == '+' || c == '^' || c == '&' || c == '*' || c == '.' || c == '[' || c == ']' || c == '!' ||
		c == '<' || c == '>' || c == '|' || c == '-' || c == '/' || c == '%'
}

func GetSliceFromLocation(location Location, source []rune) []rune {
	return source[location.Start : location.End+1]
}
