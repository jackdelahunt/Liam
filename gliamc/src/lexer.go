package src

type TokenType int16
type OperatorType int8

const (
	OP_PLUS OperatorType = iota
	OP_MULT
	OP_MINUS
	OP_DIV
)

const (
	TOKEN_NUMBER_LITERAL TokenType = iota // 0
	TOKEN_STRING_LITERAL                  // "hello"
	TOKEN_IDENTIFIER                      // x
	TOKEN_LET                             // let
	TOKEN_INSERT                          // insert
	TOKEN_FN                              // fn
	TOKEN_PAREN_OPEN                      // (
	TOKEN_PAREN_CLOSE                     // )
	TOKEN_BRACE_OPEN                      // {
	TOKEN_BRACE_CLOSE                     // }
	TOKEN_PLUS                            // +
	TOKEN_MINUS                           // -
	TOKEN_STAR                            // *
	TOKEN_SLASH                           // /
	TOKEN_MOD                             // %
	TOKEN_ASSIGN                          // =
	TOKEN_SEMI_COLON                      // ;
	TOKEN_COMMA                           // ,
	TOKEN_COLON                           // :
	TOKEN_RETURN                          // return
	TOKEN_HAT                             // ^
	TOKEN_AT                              // @
	TOKEN_STRUCT                          // struct
	TOKEN_DOT                             // .
	TOKEN_NEW                             // new
	TOKEN_BREAK                           // break
	TOKEN_IMPORT                          // import
	TOKEN_BRACKET_OPEN                    // [
	TOKEN_BRACKET_CLOSE                   // ]
	TOKEN_FOR                             // for
	TOKEN_FALSE                           // false
	TOKEN_TRUE                            // true
	TOKEN_IF                              // if
	TOKEN_ELSE                            // else
	TOKEN_OR                              // or
	TOKEN_AND                             // and
	TOKEN_EQUAL                           // ==
	TOKEN_NOT_EQUAL                       // !=
	TOKEN_NOT                             // !
	TOKEN_LESS                            // <
	TOKEN_GREATER                         // >
	TOKEN_GREATER_EQUAL                   // >=
	TOKEN_LESS_EQUAL                      // <=
	TOKEN_EXTERN                          // extern
	TOKEN_BAR                             // |
	TOKEN_IS                              // is
	TOKEN_NULL                            // null
	TOKEN_ENUM                            // enum
	TOKEN_CONTINUE                        // continue
	TOKEN_ALIAS                           // alias
	TOKEN_AS                              // as
	TOKEN_TEST                            // test
	TOKEN_ZERO                            // zero
)

type Token struct {
	tokenType   TokenType
	tokenString string
}
