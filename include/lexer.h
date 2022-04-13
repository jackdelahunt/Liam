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
        STRING_LITERAL,
        IDENTIFIER,
        LET,
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

    bool is_delim(char c) {
        return c == ' ' || c == '\n';
    }

    struct Lexer
    {
        std::vector<Token> tokens;
        std::vector<char> chars;
        int current;

        Lexer() {
            tokens = std::vector<Token>();
            current = 0;
        }

        void lex(const char* path) {
            chars = extract_chars(path);
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
                    case '"':
                        {
                            current++; // go to next character after quote
                            std::string str = std::string();
                            while(current < chars.size() && chars.at(current) != '"') {
                                str.append(std::string(1, chars.at(current)));
                                current++;
                            }
                            tokens.push_back(Token{TokenType::STRING_LITERAL, str});
                        }
                        break;
                    default:
                        auto word = get_word();

                        // check keywords
                        if(word == "let"){
                            tokens.push_back(Token{TokenType::LET, word});
                            continue;
                        }
                        
                        // check numbers
                        try
                        {
                            int i = std::stoi(word);
                            tokens.push_back(Token{TokenType::INT_LITERAL, word});
                            continue;
                        } catch(const std::exception& e){}
                        
                        
                        tokens.push_back(Token{TokenType::IDENTIFIER, word});
                        break;
                }
            }
        }

        std::string get_word() {
            std::string word = std::string();
            while(current < chars.size() && !is_delim(chars.at(current))) {
                word.append(1, chars.at(current));
                current++;
            }

            return word;
        }
    };
}
