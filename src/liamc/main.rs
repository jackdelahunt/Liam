use std::{fs, collections::HashMap};
use std::fs::File;
use std::io::prelude::*;

pub struct FileParser;

fn main() {
    let source = fs::read_to_string("main.liam")
        .expect("Something went wrong reading the file");

    let mut lexer = Lexer::new(source);
    
    lexer.lex();
    for token in lexer.tokens {
        println!("{:?}", token);
    }
}

#[derive(Debug)]
enum TokenType {
    IntLiteral,
    StringLiteral,
    LeftBrace,
    RightBrace,
    Let,
}

#[derive(Debug)]
struct Token {
    token_type: TokenType,
    string: String
}

impl Token {
    fn new(token_type: TokenType, string: String) -> Self {
        return Self{token_type, string};
    }
}

struct Lexer {
    tokens: Vec<Token>,
    source: String,
    current: usize
}

impl Lexer {
    fn new(source: String) -> Self {
        return Self{tokens: Vec::new(), source, current: 0};
    }

    fn lex(&mut self) {
        while !self.is_at_end() {
            match self.peek_char() {
                '\n' => {},
                '\r' => {},
                '\t' => {},
                ' ' => {},
                '{' => {
                    let token = Token::new(TokenType::LeftBrace, String::from('{'));
                    self.tokens.push(token)
                },
                '}' => {
                    let token = Token::new(TokenType::RightBrace, String::from('}'));
                    self.tokens.push(token)
                },
                _ => {
                    let word = self.next_word();
                    match word.as_str() {
                        "let" => {
                            let token = Token::new(TokenType::Let, word);
                            self.tokens.push(token)
                        },
                        _ => {
                            if let Ok(_) = word.parse::<i32>() {
                                let token = Token::new(TokenType::IntLiteral, word);
                                self.tokens.push(token)
                            } else {
                                let token = Token::new(TokenType::StringLiteral, word);
                                self.tokens.push(token)
                            }
                        }
                    }
                }
            }

            self.current += 1;
        }
    }

    fn peek_char(&mut self) -> char {
        let c = self.source.chars().nth(self.current).unwrap();
        return c;
    }

    fn next_char(&mut self) -> char {
        let c = self.source.chars().nth(self.current).unwrap();
        self.current += 1;
        return c;
    }

    fn next_word(&mut self) -> String {
        let source_as_chars: Vec<char> = self.source.chars().collect();
        let start_index = self.current;
        while !self.is_at_end() && char::is_alphanumeric(source_as_chars[self.current]) {
            self.current += 1;
        }

        let slice = &self.source[start_index..self.current];
        return String::from(slice);
    }

    fn is_at_end(&self) -> bool {
        return self.source.len() <= self.current;
    }
}

