#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

namespace liam {

    std::vector<char> extract_chars(const char* path) {
        auto vec = std::vector<char>();

        std::ifstream file;  
        file.open(path);
        
        for (int i = file.get(); i != EOF; i = file.get())
        {
            vec.push_back((char)i);
        }

        file.close();
        return vec;
    }

    enum TokenType {
        INT_LITERAL,
        IDENTIFIER,
        PLUS
    };

    struct Token
    {
        TokenType type;
        std::string string;
    };
    
    std::ostream& operator<<(std::ostream& os, const Token& token)
    {
        os << token.type << " : " << token.string << "\n";
        return os;
    }

    struct Lexer
    {
        std::vector<Token> tokens;
        int current;

        Lexer() {
            tokens = std::vector<Token>();
            current = 0;
        }

        void lex(const char* path) {
            auto chars = extract_chars(path);
            for(; current < chars.size(); current++) {
                char c = chars.at(current); 
                switch (c)
                {
                    case ' ':
                    case '\r':
                    case '\n':
                    case '\t':
                    break;
                    case '+':
                        tokens.push_back(Token{TokenType::PLUS, "+"});
                        break;
                    case '#':
                        while(current < chars.size() && chars.at(current) != '\n') {
                            current++;
                        }
                        break;
                    default:
                        if(std::isdigit(c)) {
                            std::string literal = std::string();
                            while(current < chars.size() && std::isdigit(chars.at(current))) {
                                literal.append(std::string(1, chars.at(current)));
                                current++;
                            }
                            current--; // reset back to non-digit
                            tokens.push_back(Token{TokenType::INT_LITERAL, literal});
                        } else {
                            std::string identifier = std::string();
                            while(current < chars.size() && std::isalpha(chars.at(current))) {
                                identifier.append(std::string(1, chars.at(current)));
                                current++;
                            }
                            current--; // reset back to non-alpha
                            tokens.push_back(Token{TokenType::IDENTIFIER, identifier});
                        }
                    break;
                }
            }
        }
    };
}
