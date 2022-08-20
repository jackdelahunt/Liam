#include "lib.h"

// enum forward declarations
enum class TokenType;
// struct forward declarations
template <typename O, typename T> struct Tuple;
struct Token;
struct Lexer;
struct IsExpression;
struct BinaryExpression;
struct UnaryExpression;
struct NumberLiteralExpression;
struct StringLiteralExpression;
struct BoolLiteralExpression;
struct IdentifierExpression;
struct CallExpression;
struct GetExpression;
struct NewExpression;
struct GroupExpression;
struct NullLiteralExpression;
struct IdentifierTypeExpression;
struct UnaryTypeExpression;
struct UnionTypeExpression;
struct SpecifiedGenericsTypeExpression;
struct ExpressionStatement;
struct LetStatement;
struct ScopeStatement;
struct FnStatement;
struct StructStatement;
struct AssignmentStatement;
struct InsertStatement;
struct ImportStatement;
struct ForStatement;
struct IfStatement;
struct ElseStatement;
struct ReturnStatement;
struct BreakStatement;
struct EnumStatement;
struct ContinueStatement;
struct AliasStatement;
struct File;
struct Parser;
// typedefs
typedef std::variant<
    IsExpression *, BinaryExpression *, UnaryExpression *, NumberLiteralExpression *, StringLiteralExpression *,
    BoolLiteralExpression *, IdentifierExpression *, CallExpression *, GetExpression *, NewExpression *,
    GroupExpression *, NullLiteralExpression *>
    Expression;
typedef std::variant<
    IdentifierTypeExpression *, UnaryTypeExpression *, UnionTypeExpression *, SpecifiedGenericsTypeExpression *>
    TypeExpression;
typedef std::variant<
    ExpressionStatement, LetStatement, ScopeStatement, FnStatement, StructStatement, AssignmentStatement,
    InsertStatement, ImportStatement, ForStatement, IfStatement, ElseStatement, ReturnStatement, BreakStatement,
    EnumStatement, ContinueStatement, AliasStatement>
    Statement;

// function forward declarations
void __liam__main__();
Lexer make_lexer(str path);
void next_char(Lexer *lexer);
str peek(Lexer *lexer);
void lex(Lexer *lexer);
str get_word(Lexer *lexer);
bool is_delim(str c);
bool match(Parser *parser, TokenType type);
Token parser_peek(Parser *parser, u64 offset);

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
    ALIAS,
    AS,
};
std::ostream &operator<<(std::ostream &os, const TokenType &obj) {
    switch (obj)
    {
    case TokenType::NUMBER_LITERAL:
        os << "TokenType.NUMBER_LITERAL";
        break;
    case TokenType::STRING_LITERAL:
        os << "TokenType.STRING_LITERAL";
        break;
    case TokenType::IDENTIFIER:
        os << "TokenType.IDENTIFIER";
        break;
    case TokenType::LET:
        os << "TokenType.LET";
        break;
    case TokenType::INSERT:
        os << "TokenType.INSERT";
        break;
    case TokenType::FN:
        os << "TokenType.FN";
        break;
    case TokenType::PAREN_OPEN:
        os << "TokenType.PAREN_OPEN";
        break;
    case TokenType::PAREN_CLOSE:
        os << "TokenType.PAREN_CLOSE";
        break;
    case TokenType::BRACE_OPEN:
        os << "TokenType.BRACE_OPEN";
        break;
    case TokenType::BRACE_CLOSE:
        os << "TokenType.BRACE_CLOSE";
        break;
    case TokenType::PLUS:
        os << "TokenType.PLUS";
        break;
    case TokenType::MINUS:
        os << "TokenType.MINUS";
        break;
    case TokenType::STAR:
        os << "TokenType.STAR";
        break;
    case TokenType::SLASH:
        os << "TokenType.SLASH";
        break;
    case TokenType::MOD:
        os << "TokenType.MOD";
        break;
    case TokenType::ASSIGN:
        os << "TokenType.ASSIGN";
        break;
    case TokenType::SEMI_COLON:
        os << "TokenType.SEMI_COLON";
        break;
    case TokenType::COMMA:
        os << "TokenType.COMMA";
        break;
    case TokenType::COLON:
        os << "TokenType.COLON";
        break;
    case TokenType::RETURN:
        os << "TokenType.RETURN";
        break;
    case TokenType::HAT:
        os << "TokenType.HAT";
        break;
    case TokenType::AT:
        os << "TokenType.AT";
        break;
    case TokenType::STRUCT:
        os << "TokenType.STRUCT";
        break;
    case TokenType::DOT:
        os << "TokenType.DOT";
        break;
    case TokenType::NEW:
        os << "TokenType.NEW";
        break;
    case TokenType::BREAK:
        os << "TokenType.BREAK";
        break;
    case TokenType::IMPORT:
        os << "TokenType.IMPORT";
        break;
    case TokenType::BRACKET_OPEN:
        os << "TokenType.BRACKET_OPEN";
        break;
    case TokenType::BRACKET_CLOSE:
        os << "TokenType.BRACKET_CLOSE";
        break;
    case TokenType::FOR:
        os << "TokenType.FOR";
        break;
    case TokenType::FALSE:
        os << "TokenType.FALSE";
        break;
    case TokenType::TRUE:
        os << "TokenType.TRUE";
        break;
    case TokenType::IF:
        os << "TokenType.IF";
        break;
    case TokenType::ELSE:
        os << "TokenType.ELSE";
        break;
    case TokenType::OR:
        os << "TokenType.OR";
        break;
    case TokenType::AND:
        os << "TokenType.AND";
        break;
    case TokenType::EQUAL:
        os << "TokenType.EQUAL";
        break;
    case TokenType::NOT_EQUAL:
        os << "TokenType.NOT_EQUAL";
        break;
    case TokenType::NOT:
        os << "TokenType.NOT";
        break;
    case TokenType::LESS:
        os << "TokenType.LESS";
        break;
    case TokenType::GREATER:
        os << "TokenType.GREATER";
        break;
    case TokenType::GREATER_EQUAL:
        os << "TokenType.GREATER_EQUAL";
        break;
    case TokenType::LESS_EQUAL:
        os << "TokenType.LESS_EQUAL";
        break;
    case TokenType::EXTERN:
        os << "TokenType.EXTERN";
        break;
    case TokenType::BAR:
        os << "TokenType.BAR";
        break;
    case TokenType::IS:
        os << "TokenType.IS";
        break;
    case TokenType::NULL_POINTER:
        os << "TokenType.NULL_POINTER";
        break;
    case TokenType::ENUM:
        os << "TokenType.ENUM";
        break;
    case TokenType::CONTINUE:
        os << "TokenType.CONTINUE";
        break;
    case TokenType::ALIAS:
        os << "TokenType.ALIAS";
        break;
    case TokenType::AS:
        os << "TokenType.AS";
        break;
    }
    return os;
}

template <typename O, typename T> struct Tuple {
    O one;
    T two;
};
struct Token {
    TokenType type;
    str string;
};
struct Lexer {
    str path;
    String source;
    u64 current;
    Array<Token> tokens;
};
struct IsExpression {
    Expression expression;
    Token identifier;
};
struct BinaryExpression {
    Expression *left;
    Token op;
    Expression *right;
};
struct UnaryExpression {
    Token op;
    Expression *expression;
};
struct NumberLiteralExpression {
    Token token;
    s64 number;
};
struct StringLiteralExpression {
    Token token;
};
struct BoolLiteralExpression {
    Token value;
};
struct IdentifierExpression {
    Token identifier;
};
struct CallExpression {
    Expression *identifier;
    Array<Expression *> args;
    Array<Expression *> generics;
};
struct GetExpression {
    Expression *lhs;
    Token member;
};
struct NewExpression {
    Token identifier;
    Array<TypeExpression *> generics;
    Array<Tuple<Token, Expression *>> named_expressions;
};
struct GroupExpression {
    Expression *expression;
};
struct NullLiteralExpression {};
struct IdentifierTypeExpression {
    Token identifier;
};
struct UnaryTypeExpression {
    Token op;
    TypeExpression *type_expression;
};
struct UnionTypeExpression {
    Array<TypeExpression *> type_expressions;
};
struct SpecifiedGenericsTypeExpression {
    IdentifierTypeExpression *struct_type;
    Array<TypeExpression *> generics;
};
struct ExpressionStatement {
    Expression *expression;
};
struct LetStatement {
    Token identifier;
    Expression *rhs;
    TypeExpression *type;
};
struct ScopeStatement {
    Array<Statement *> statements;
};
struct FnStatement {
    Token identifier;
    Array<Token> generics;
    Array<Tuple<Token, TypeExpression *>> params;
};
struct StructStatement {
    Token identifier;
    Array<Token> generics;
    Array<Tuple<Token, TypeExpression *>> members;
    bool is_extern;
};
struct AssignmentStatement {
    Expression *lhs;
};
struct InsertStatement {
    Expression *byte_code;
};
struct ImportStatement {
    Expression *path;
};
struct ForStatement {
    Statement *assign;
    Expression *condition;
    Statement *update;
    ScopeStatement *body;
};
struct IfStatement {
    Expression *expression;
    ScopeStatement *body;
    ElseStatement *else_statement;
};
struct ElseStatement {
    IfStatement *if_statement;
    ScopeStatement *body;
};
struct ReturnStatement {
    Expression *expression;
};
struct BreakStatement {
    Token identifier;
};
struct EnumStatement {
    Token identifier;
    Array<Token> instnaces;
};
struct ContinueStatement {};
struct AliasStatement {
    Token identifier;
    TypeExpression *type_expression;
};
struct File {
    Array<Statement *> statements;
    Array<str> imports;
    String path;
};
struct Parser {
    Array<Token> tokens;
    u64 current;
    String path;
    File *file;
};
void __liam__main__() {
    set_allocator(make_str((char *)"malloc", 6), _u64(0));
    auto lexer = make_lexer(make_str((char *)"/Users/jackdelahunt/Projects/Liam/liamc/selfhost/test.liam", 58));
    lex(&(lexer));
    for (auto i = _u64(0); i < lexer.tokens.length; i = i + _u64(1))
    {
        auto t = array_index<Token>(&(lexer.tokens), i);
        println<str>(t.string);
    }
}

Lexer make_lexer(str path) {
    return Lexer{path, read(path), _u64(0), make_array<Token>()};
}

void next_char(Lexer *lexer) {
    lexer->current = lexer->current + _u64(1);
}

str peek(Lexer *lexer) {
    return substr(lexer->source.string, lexer->current + _u64(1), _u64(1));
}

void lex(Lexer *lexer) {
    for (lexer->current = _u64(0); lexer->current < len(lexer->source.string); next_char(lexer))
    {
        auto current_char = substr(lexer->source.string, lexer->current, _u64(1));
        if (current_char == make_str((char *)"\n", 1) || current_char == make_str((char *)" ", 1) ||
            current_char == make_str((char *)"\r", 1) || current_char == make_str((char *)"\t", 1))
        { continue; }

        if (current_char == make_str((char *)"+", 1))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::PLUS, make_str((char *)"+", 1)});
            continue;
        }

        if (current_char == make_str((char *)"-", 1))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::MINUS, make_str((char *)"-", 1)});
            continue;
        }

        if (current_char == make_str((char *)"*", 1))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::STAR, make_str((char *)"*", 1)});
            continue;
        }

        if (current_char == make_str((char *)"/", 1))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::SLASH, make_str((char *)"/", 1)});
            continue;
        }

        if (current_char == make_str((char *)"%", 1))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::MOD, make_str((char *)"%", 1)});
            continue;
        }

        if (current_char == make_str((char *)"=", 1))
        {
            if (peek(lexer) == make_str((char *)"=", 1))
            {
                next_char(lexer);
                array_append<Token>(&(lexer->tokens), Token{TokenType::EQUAL, make_str((char *)"==", 2)});
                continue;
            }

            array_append<Token>(&(lexer->tokens), Token{TokenType::ASSIGN, make_str((char *)"=", 1)});
            continue;
        }

        if (current_char == make_str((char *)";", 1))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::SEMI_COLON, make_str((char *)";", 1)});
            continue;
        }

        if (current_char == make_str((char *)"(", 1))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::PAREN_OPEN, make_str((char *)"(", 1)});
            continue;
        }

        if (current_char == make_str((char *)")", 1))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::PAREN_CLOSE, make_str((char *)")", 1)});
            continue;
        }

        if (current_char == make_str((char *)"{", 1))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::BRACE_OPEN, make_str((char *)"{", 1)});
            continue;
        }

        if (current_char == make_str((char *)"}", 1))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::BRACE_CLOSE, make_str((char *)"}", 1)});
            continue;
        }

        if (current_char == make_str((char *)",", 1))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::COMMA, make_str((char *)",", 1)});
            continue;
        }

        if (current_char == make_str((char *)"[", 1))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::BRACKET_OPEN, make_str((char *)"[", 1)});
            continue;
        }

        if (current_char == make_str((char *)"]", 1))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::BRACKET_CLOSE, make_str((char *)"]", 1)});
            continue;
        }

        if (current_char == make_str((char *)":", 1))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::COLON, make_str((char *)":", 1)});
            continue;
        }

        if (current_char == make_str((char *)"^", 1))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::HAT, make_str((char *)"^", 1)});
            continue;
        }

        if (current_char == make_str((char *)"@", 1))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::AT, make_str((char *)"@", 1)});
            continue;
        }

        if (current_char == make_str((char *)".", 1))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::DOT, make_str((char *)".", 1)});
            continue;
        }

        if (current_char == make_str((char *)"<", 1))
        {
            if (peek(lexer) == make_str((char *)"=", 1))
            {
                next_char(lexer);
                array_append<Token>(&(lexer->tokens), Token{TokenType::LESS_EQUAL, make_str((char *)"<=", 2)});
                continue;
            }

            array_append<Token>(&(lexer->tokens), Token{TokenType::LESS, make_str((char *)"<", 1)});
            continue;
        }

        if (current_char == make_str((char *)">", 1))
        {
            if (peek(lexer) == make_str((char *)"=", 1))
            {
                next_char(lexer);
                array_append<Token>(&(lexer->tokens), Token{TokenType::GREATER_EQUAL, make_str((char *)">=", 2)});
                continue;
            }

            array_append<Token>(&(lexer->tokens), Token{TokenType::GREATER, make_str((char *)">", 1)});
            continue;
        }

        if (current_char == make_str((char *)"|", 1))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::BAR, make_str((char *)"|", 1)});
            continue;
        }

        if (current_char == make_str((char *)"!", 1))
        {
            if (peek(lexer) == make_str((char *)"=", 1))
            {
                next_char(lexer);
                array_append<Token>(&(lexer->tokens), Token{TokenType::NOT_EQUAL, make_str((char *)"!=", 2)});
                continue;
            }

            array_append<Token>(&(lexer->tokens), Token{TokenType::NOT, make_str((char *)"!", 1)});
            continue;
        }

        if (current_char == make_str((char *)"#", 1))
        {
            for (auto c = current_char;
                 lexer->current < len(lexer->source.string) &&
                 substr(lexer->source.string, lexer->current, _u64(1)) != make_str((char *)"\n", 1);
                 next_char(lexer))
            {}

            continue;
        }

        if (current_char == make_str((char *)"\"", 1))
        {
            next_char(lexer);
            auto start = lexer->current;
            for (auto c = _s64(0); lexer->current < len(lexer->source.string) &&
                                   substr(lexer->source.string, lexer->current, _u64(1)) != make_str((char *)"\"", 1);
                 next_char(lexer))
            {}

            array_append<Token>(
                &(lexer->tokens),
                Token{TokenType::STRING_LITERAL, substr(lexer->source.string, start, lexer->current - start)}
            );
            continue;
        }

        auto word = get_word(lexer);
        if (word == make_str((char *)"let", 3))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::LET, word});
            continue;
        }

        if (word == make_str((char *)"insert", 6))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::INSERT, word});
            continue;
        }

        if (word == make_str((char *)"fn", 2))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::FN, word});
            continue;
        }

        if (word == make_str((char *)"return", 6))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::RETURN, word});
            continue;
        }

        if (word == make_str((char *)"struct", 6))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::STRUCT, word});
            continue;
        }

        if (word == make_str((char *)"new", 3))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::NEW, word});
            continue;
        }

        if (word == make_str((char *)"continue", 8))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::CONTINUE, word});
            continue;
        }

        if (word == make_str((char *)"import", 6))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::IMPORT, word});
            continue;
        }

        if (word == make_str((char *)"for", 3))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::FOR, word});
            continue;
        }

        if (word == make_str((char *)"if", 2))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::IF, word});
            continue;
        }

        if (word == make_str((char *)"else", 4))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::ELSE, word});
            continue;
        }

        if (word == make_str((char *)"and", 3))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::AND, word});
            continue;
        }

        if (word == make_str((char *)"or", 2))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::OR, word});
            continue;
        }

        if (word == make_str((char *)"extern", 6))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::EXTERN, word});
            continue;
        }

        if (word == make_str((char *)"is", 2))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::IS, word});
            continue;
        }

        if (word == make_str((char *)"enum", 4))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::ENUM, word});
            continue;
        }

        if (word == make_str((char *)"true", 4))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::TRUE, word});
            continue;
        }

        if (word == make_str((char *)"false", 5))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::FALSE, word});
            continue;
        }

        if (word == make_str((char *)"null", 4))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::NULL_POINTER, word});
            continue;
        }

        if (word == make_str((char *)"alias", 5))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::ALIAS, word});
            continue;
        }

        if (word == make_str((char *)"as", 2))
        {
            array_append<Token>(&(lexer->tokens), Token{TokenType::AS, word});
            continue;
        }

        array_append<Token>(&(lexer->tokens), Token{TokenType::IDENTIFIER, word});
    }
}

str get_word(Lexer *lexer) {
    auto start = lexer->current;
    for (auto c = _s64(0); lexer->current < len(lexer->source.string) &&
                           !(is_delim(substr(lexer->source.string, lexer->current, _u64(1))));
         next_char(lexer))
    {}

    auto word      = substr(lexer->source.string, start, lexer->current - start);
    lexer->current = lexer->current - _u64(1);
    return word;
}

bool is_delim(str c) {
    return c == make_str((char *)" ", 1) || c == make_str((char *)"\n", 1) || c == make_str((char *)";", 1) ||
           c == make_str((char *)"(", 1) || c == make_str((char *)")", 1) || c == make_str((char *)"{", 1) ||
           c == make_str((char *)"}", 1) || c == make_str((char *)",", 1) || c == make_str((char *)":", 1) ||
           c == make_str((char *)"=", 1) || c == make_str((char *)"+", 1) || c == make_str((char *)"^", 1) ||
           c == make_str((char *)"@", 1) || c == make_str((char *)"*", 1) || c == make_str((char *)".", 1) ||
           c == make_str((char *)"[", 1) || c == make_str((char *)"]", 1) || c == make_str((char *)"!", 1) ||
           c == make_str((char *)"<", 1) || c == make_str((char *)">", 1) || c == make_str((char *)"|", 1) ||
           c == make_str((char *)"-", 1) || c == make_str((char *)"/", 1) || c == make_str((char *)"%", 1);
}

bool match(Parser *parser, TokenType type) {
    if (parser->tokens.length > _u64(0))
    {}

    return false;
}

Token parser_peek(Parser *parser, u64 offset) {
    return array_index<Token>(&(parser->tokens), parser->current + offset);
}

int main(int argc, char **argv) {
    __liam__main__();
}