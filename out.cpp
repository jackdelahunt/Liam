#include "lib.h"

// enum forward declarations
enum class TokenType;
// struct forward declarations
struct Token;
struct Lexer;

// function forward declarations
void  __liam__main__();

// Source
enum class TokenType {
Let,
 };
std::ostream& operator<<(std::ostream& os, const TokenType &obj) {
switch(obj) {
case TokenType::Let: os << "TokenType.Let"; break;
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
os << "}" << std::endl;
return os;
}

struct Lexer {
String source;
u64 current;};
std::ostream& operator<<(std::ostream& os, const Lexer &obj) {
os << "Lexer" << " {" << std::endl;
os << "   " << "source: " << obj.source << "," << std::endl;
os << "   " << "current: " << obj.current << "," << std::endl;
os << "}" << std::endl;
return os;
}

void __liam__main__(){
auto source = read(make_str((char*)"/Users/jackdelahunt/Projects/Liam/liamc/selfhost/test.liam", 58));
println<String>(source);

}

int main(int argc, char **argv) { __liam__main__(); }
