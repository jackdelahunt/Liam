#[derive(Debug)]
pub enum TokenType {
    IntLiteral,
    Identifier,
    LeftBrace,
    RightBrace,
    Plus,
    Let,
}

#[derive(Debug)]
pub struct Token {
    token_type: TokenType,
    string: String,
    start: usize,
    end: usize
}

impl Token {
    fn new(token_type: TokenType, string: String, start: usize, end: usize) -> Self {
        return Self{token_type, string, start, end};
    }
}

pub struct Lexer {
    pub tokens: Vec<Token>,
    source: String,
    current: usize
}

impl Lexer {
    pub fn new(source: String) -> Self {
        return Self{tokens: Vec::new(), source, current: 0};
    }

    pub fn lex(&mut self) {
        while !self.is_at_end() {
            match self.peek_char() {
                '\n' => {},
                '\r' => {},
                '\t' => {},
                ' ' => {},
                '{' => {
                    let token = Token::new(
                        TokenType::LeftBrace, 
                        String::from('{'), 
                        self.current, 
                        self.current + 1);

                    self.tokens.push(token)
                },
                '}' => {
                    let token = Token::new(
                        TokenType::RightBrace, 
                        String::from('}'), 
                        self.current, 
                        self.current + 1);

                    self.tokens.push(token)
                },'+' => {
                    let token = Token::new(
                        TokenType::Plus, 
                        String::from('+'), 
                        self.current, 
                        self.current + 1);

                    self.tokens.push(token)
                },
                _ => {
                    let word = self.next_word();
                    match word.as_str() {
                        "let" => {
                            let start = self.current - word.len();
                            let token = Token::new(
                                TokenType::Let, 
                                word,
                                start,
                                self.current
                            );

                            self.tokens.push(token)
                        },
                        _ => {
                            if let Ok(_) = word.parse::<i32>() {
                                let start = self.current - word.len();
                                let token = Token::new(
                                    TokenType::IntLiteral, 
                                    word,
                                    start,
                                    self.current
                                );

                                self.tokens.push(token)
                            } else {
                                let start = self.current - word.len();
                                let token = Token::new(
                                    TokenType::Identifier, 
                                    word,
                                    start,
                                    self.current
                                );
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

