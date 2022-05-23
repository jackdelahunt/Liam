pub enum TokenType {
    Identifier(String),
    Number(u32),
    Let,
    Assign,
    SemiColon,
}

pub struct Token {
    token_type: TokenType,
    span: Span
}

impl Token {
    pub fn new(token_type: TokenType, span: Span) -> Self {
        return Self {
            token_type,
            span
        };
    }
}

pub struct Lexer {
    index: usize,
    chars: Vec<char>
}

impl Lexer {
    pub fn new() -> Self {
        return Self {
            index: 0,
            chars: vec![]
        };
    }

    pub fn lex(mut self, source: String) -> Vec<Token>  {
        let mut tokens: Vec<Token> = vec![];
        let source_chars: Vec<char> = source.chars().collect();
    
        while self.index < source.len() {
            let current = source_chars[self.index];
            let mut is_single = true;
            
            match current {
                '=' => tokens.push(Token::new(TokenType::Assign,Span::new(self.index, self.index + 1))),
                ';' => tokens.push(Token::new(TokenType::SemiColon,Span::new(self.index, self.index + 1))),
                _ => is_single = false,
            }
    
            if is_single {
                self.index += 1;
                continue;
            }

            let word = self.get_word();
            

            if self.compare(&word, "let") {
                tokens.push(Token::new(TokenType::Let,word.clone()));
                self.index += word.len();
                continue;
            }
    
        }
    
        return tokens;
    }

    pub fn get_word(&self) -> Span {
        let start = self.index;
        let mut end = start;
        while end < self.chars.len() && !is_delim(self.chars[end]){
            end += 1;
        }

        return Span::new(start, end);
    }

    fn compare(&self, span: &Span, string: &'static str) -> bool {
        if span.end - span.start != string.len() {
            return false;
        }


        let slice1 = &self.chars[span.start..span.end];
    
        let string_chars: Vec<char> = string.chars().collect(); 
        let slice2 = &string_chars[0..string_chars.len()];
    
        return slice1 == slice2;
    }

}

fn is_delim(c: char) -> bool {
    return c == ' ' || c == ';';
}

#[derive(Clone)]
pub struct Span {
    start: usize,
    end: usize
}

impl Span {
    pub fn new(start: usize, end: usize) -> Self {
        return Self {start, end};
    }

    pub fn len(&self) -> usize {
        return self.end - self.start;
    }

}