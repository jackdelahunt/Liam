#include "lib.h"

// enum forward declarations
enum class TokenType;
// struct forward declarations
struct Token;
struct Lexer;

// function forward declarations
Lexer make_lexer(str path);
void next_char(Lexer* lexer);
str peek(Lexer* lexer);
void lex(Lexer* lexer);
void  __liam__main__();

// Source
enum class TokenType {
NUMBER_LITERAL,
STRING_LITERAL,
IDENTIFIER,
LET,
INSERT,
FN,
PAREN_OPEN,
PAREN_CLOSE,
BRACE_OPEN,
BRACE_CLOSE,
PLUS,
MINUS,
STAR,
SLASH,
MOD,
ASSIGN,
SEMI_COLON,
COMMA,
COLON,
RETURN,
HAT,
AT,
STRUCT,
DOT,
NEW,
BREAK,
IMPORT,
BRACKET_OPEN,
BRACKET_CLOSE,
FOR,
FALSE,
TRUE,
IF,
ELSE,
OR,
AND,
EQUAL,
NOT_EQUAL,
NOT,
LESS,
GREATER,
GREATER_EQUAL,
LESS_EQUAL,
EXTERN,
BAR,
IS,
NULL_POINTER,
ENUM,
CONTINUE,
 };
std::ostream& operator<<(std::ostream& os, const TokenType &obj) {
switch(obj) {
case TokenType::NUMBER_LITERAL: os << "TokenType.NUMBER_LITERAL"; break;
case TokenType::STRING_LITERAL: os << "TokenType.STRING_LITERAL"; break;
case TokenType::IDENTIFIER: os << "TokenType.IDENTIFIER"; break;
case TokenType::LET: os << "TokenType.LET"; break;
case TokenType::INSERT: os << "TokenType.INSERT"; break;
case TokenType::FN: os << "TokenType.FN"; break;
case TokenType::PAREN_OPEN: os << "TokenType.PAREN_OPEN"; break;
case TokenType::PAREN_CLOSE: os << "TokenType.PAREN_CLOSE"; break;
case TokenType::BRACE_OPEN: os << "TokenType.BRACE_OPEN"; break;
case TokenType::BRACE_CLOSE: os << "TokenType.BRACE_CLOSE"; break;
case TokenType::PLUS: os << "TokenType.PLUS"; break;
case TokenType::MINUS: os << "TokenType.MINUS"; break;
case TokenType::STAR: os << "TokenType.STAR"; break;
case TokenType::SLASH: os << "TokenType.SLASH"; break;
case TokenType::MOD: os << "TokenType.MOD"; break;
case TokenType::ASSIGN: os << "TokenType.ASSIGN"; break;
case TokenType::SEMI_COLON: os << "TokenType.SEMI_COLON"; break;
case TokenType::COMMA: os << "TokenType.COMMA"; break;
case TokenType::COLON: os << "TokenType.COLON"; break;
case TokenType::RETURN: os << "TokenType.RETURN"; break;
case TokenType::HAT: os << "TokenType.HAT"; break;
case TokenType::AT: os << "TokenType.AT"; break;
case TokenType::STRUCT: os << "TokenType.STRUCT"; break;
case TokenType::DOT: os << "TokenType.DOT"; break;
case TokenType::NEW: os << "TokenType.NEW"; break;
case TokenType::BREAK: os << "TokenType.BREAK"; break;
case TokenType::IMPORT: os << "TokenType.IMPORT"; break;
case TokenType::BRACKET_OPEN: os << "TokenType.BRACKET_OPEN"; break;
case TokenType::BRACKET_CLOSE: os << "TokenType.BRACKET_CLOSE"; break;
case TokenType::FOR: os << "TokenType.FOR"; break;
case TokenType::FALSE: os << "TokenType.FALSE"; break;
case TokenType::TRUE: os << "TokenType.TRUE"; break;
case TokenType::IF: os << "TokenType.IF"; break;
case TokenType::ELSE: os << "TokenType.ELSE"; break;
case TokenType::OR: os << "TokenType.OR"; break;
case TokenType::AND: os << "TokenType.AND"; break;
case TokenType::EQUAL: os << "TokenType.EQUAL"; break;
case TokenType::NOT_EQUAL: os << "TokenType.NOT_EQUAL"; break;
case TokenType::NOT: os << "TokenType.NOT"; break;
case TokenType::LESS: os << "TokenType.LESS"; break;
case TokenType::GREATER: os << "TokenType.GREATER"; break;
case TokenType::GREATER_EQUAL: os << "TokenType.GREATER_EQUAL"; break;
case TokenType::LESS_EQUAL: os << "TokenType.LESS_EQUAL"; break;
case TokenType::EXTERN: os << "TokenType.EXTERN"; break;
case TokenType::BAR: os << "TokenType.BAR"; break;
case TokenType::IS: os << "TokenType.IS"; break;
case TokenType::NULL_POINTER: os << "TokenType.NULL_POINTER"; break;
case TokenType::ENUM: os << "TokenType.ENUM"; break;
case TokenType::CONTINUE: os << "TokenType.CONTINUE"; break;
}
return os;
}

struct Token {
TokenType type;
str string;};
std::ostream& operator<<(std::ostream& os, const Token &obj) {
os << "Token" << " {" << std::endl;
os << "   " << "type: " << obj.type << "," << std::endl;
os << "   " << "string: " << obj.string << "," << std::endl;
os << "}";
return os;
}

struct Lexer {
str path;
String source;
u64 current;
Array<Token> tokens;};
std::ostream& operator<<(std::ostream& os, const Lexer &obj) {
os << "Lexer" << " {" << std::endl;
os << "   " << "path: " << obj.path << "," << std::endl;
os << "   " << "source: " << obj.source << "," << std::endl;
os << "   " << "current: " << obj.current << "," << std::endl;
os << "   " << "tokens: " << obj.tokens << "," << std::endl;
os << "}";
return os;
}

Lexer make_lexer(str path){
return Lexer{path, read(path), _u64(0), make_array<Token>()};
}

void next_char(Lexer* lexer){
lexer->current = lexer->current + _u64(1);

}

str peek(Lexer* lexer){
return substr(lexer->source.string, lexer->current + _u64(1), _u64(1));
}

void lex(Lexer* lexer){
for(lexer->current = _u64(0);
lexer->current < len(lexer->source.string);next_char(lexer)){
auto current_char = substr(lexer->source.string, lexer->current, _u64(1));
if (current_char == make_str((char*)"\n", 2) || current_char == make_str((char*)" ", 1) || current_char == make_str((char*)"\r", 2) || current_char == make_str((char*)"\t", 2)) {
continue;
}

if (current_char == make_str((char*)"+", 1)) {
array_append<Token>(&(lexer->tokens), Token{TokenType::PLUS, make_str((char*)"+", 1)});
continue;
}

if (current_char == make_str((char*)"-", 1)) {
array_append<Token>(&(lexer->tokens), Token{TokenType::MINUS, make_str((char*)"-", 1)});
continue;
}

if (current_char == make_str((char*)"*", 1)) {
array_append<Token>(&(lexer->tokens), Token{TokenType::STAR, make_str((char*)"*", 1)});
continue;
}

if (current_char == make_str((char*)"/", 1)) {
array_append<Token>(&(lexer->tokens), Token{TokenType::SLASH, make_str((char*)"/", 1)});
continue;
}

if (current_char == make_str((char*)"%", 1)) {
array_append<Token>(&(lexer->tokens), Token{TokenType::MOD, make_str((char*)"%", 1)});
continue;
}

if (current_char == make_str((char*)"=", 1)) {
if (peek(lexer) == make_str((char*)"=", 1)) {
next_char(lexer);
array_append<Token>(&(lexer->tokens), Token{TokenType::EQUAL, make_str((char*)"==", 2)});
continue;
}

array_append<Token>(&(lexer->tokens), Token{TokenType::ASSIGN, make_str((char*)";", 1)});
continue;
}

if (current_char == make_str((char*)";", 1)) {
array_append<Token>(&(lexer->tokens), Token{TokenType::SEMI_COLON, make_str((char*)";", 1)});
continue;
}

if (current_char == make_str((char*)"(", 1)) {
array_append<Token>(&(lexer->tokens), Token{TokenType::PAREN_OPEN, make_str((char*)"(", 1)});
continue;
}

if (current_char == make_str((char*)")", 1)) {
array_append<Token>(&(lexer->tokens), Token{TokenType::PAREN_CLOSE, make_str((char*)")", 1)});
continue;
}

if (current_char == make_str((char*)"{", 1)) {
array_append<Token>(&(lexer->tokens), Token{TokenType::BRACE_OPEN, make_str((char*)"{", 1)});
continue;
}

if (current_char == make_str((char*)"}", 1)) {
array_append<Token>(&(lexer->tokens), Token{TokenType::BRACE_CLOSE, make_str((char*)"}", 1)});
continue;
}

if (current_char == make_str((char*)",", 1)) {
array_append<Token>(&(lexer->tokens), Token{TokenType::COMMA, make_str((char*)",", 1)});
continue;
}

if (current_char == make_str((char*)"[", 1)) {
array_append<Token>(&(lexer->tokens), Token{TokenType::BRACKET_OPEN, make_str((char*)"[", 1)});
continue;
}

if (current_char == make_str((char*)"]", 1)) {
array_append<Token>(&(lexer->tokens), Token{TokenType::BRACKET_CLOSE, make_str((char*)"]", 1)});
continue;
}

if (current_char == make_str((char*)":", 1)) {
array_append<Token>(&(lexer->tokens), Token{TokenType::COLON, make_str((char*)":", 1)});
continue;
}

if (current_char == make_str((char*)"^", 1)) {
array_append<Token>(&(lexer->tokens), Token{TokenType::HAT, make_str((char*)"^", 1)});
continue;
}

if (current_char == make_str((char*)"@", 1)) {
array_append<Token>(&(lexer->tokens), Token{TokenType::AT, make_str((char*)"@", 1)});
continue;
}

if (current_char == make_str((char*)".", 1)) {
array_append<Token>(&(lexer->tokens), Token{TokenType::DOT, make_str((char*)".", 1)});
continue;
}

if (current_char == make_str((char*)"<", 1)) {
if (peek(lexer) == make_str((char*)"=", 1)) {
next_char(lexer);
array_append<Token>(&(lexer->tokens), Token{TokenType::LESS_EQUAL, make_str((char*)"<=", 2)});
continue;
}

array_append<Token>(&(lexer->tokens), Token{TokenType::LESS, make_str((char*)"<", 1)});
continue;
}

if (current_char == make_str((char*)">", 1)) {
if (peek(lexer) == make_str((char*)"=", 1)) {
next_char(lexer);
array_append<Token>(&(lexer->tokens), Token{TokenType::GREATER_EQUAL, make_str((char*)">=", 2)});
continue;
}

array_append<Token>(&(lexer->tokens), Token{TokenType::GREATER, make_str((char*)">", 1)});
continue;
}

if (current_char == make_str((char*)"|", 1)) {
array_append<Token>(&(lexer->tokens), Token{TokenType::BAR, make_str((char*)"|", 1)});
continue;
}

if (current_char == make_str((char*)"!", 1)) {
if (peek(lexer) == make_str((char*)"=", 1)) {
next_char(lexer);
array_append<Token>(&(lexer->tokens), Token{TokenType::NOT_EQUAL, make_str((char*)"!=", 2)});
continue;
}

array_append<Token>(&(lexer->tokens), Token{TokenType::NOT, make_str((char*)"!", 1)});
continue;
}

if (current_char == make_str((char*)"#", 1)) {
for(auto c = current_char;
lexer->current < len(lexer->source.string) && substr(lexer->source.string, lexer->current, _u64(1)) != make_str((char*)"\n", 2);next_char(lexer)){

}

continue;
}


}


}

void __liam__main__(){
set_allocator(make_str((char*)"malloc", 6), _u64(0));
auto lexer = make_lexer(make_str((char*)"/Users/jackdelahunt/Projects/Liam/liamc/selfhost/test.liam", 58));
lexer.current = _u64(100);
lex(&(lexer));
println<Array<Token>>(lexer.tokens);

}

int main(int argc, char **argv) { __liam__main__(); }