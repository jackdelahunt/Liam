use std::str;

#[derive(Debug)]
pub enum TokenType {
    Identifier(String),
    Number(u64),
    Let,
    Fn,
    Assign,
    SemiColon,
    ParenOpen,
    ParenClose,
    BraceOpen,
    BraceClose,
}

#[derive(Debug)]
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

pub fn lex(source: String) -> Vec<Token>  {
    let mut tokens: Vec<Token> = vec![];
    let mut index: usize = 0;
    let bytes = source.as_bytes();

    while index < source.len() {
        let current = bytes[index];
        let mut is_single = true;
        
        match current {
            b' ' | b'\n' | b'\t' => { index += 1; continue; }
            b'=' => tokens.push(Token::new(TokenType::Assign,Span::new(index, index + 1))),
            b';' => tokens.push(Token::new(TokenType::SemiColon,Span::new(index, index + 1))),
            b'(' => tokens.push(Token::new(TokenType::ParenOpen,Span::new(index, index + 1))),
            b')' => tokens.push(Token::new(TokenType::ParenClose,Span::new(index, index + 1))),
            b'{' => tokens.push(Token::new(TokenType::BraceOpen,Span::new(index, index + 1))),
            b'}' => tokens.push(Token::new(TokenType::BraceClose,Span::new(index, index + 1))),
            _ => is_single = false,
        }

        if is_single {
            index += 1;
            continue;
        }

        let word = get_word(bytes, index);
        

        if compare(&word, bytes, "let") {
            tokens.push(Token::new(TokenType::Let,word.clone()));
            index += word.len();
            continue;
        }

        if compare(&word, bytes, "fn") {
            tokens.push(Token::new(TokenType::Fn,word.clone()));
            index += word.len();
            continue;
        }

        let as_str = byte_span_string(word.clone(), bytes).expect("This blew up oh no");
        if let Ok(n) = as_str.parse::<u64>() {
            tokens.push(Token::new(TokenType::Number(n),word.clone()));
            index += word.len();
            continue;
        } 

        tokens.push(Token::new(TokenType::Identifier(as_str),word.clone()));
        index += word.len();

    }

    return tokens;
}

fn byte_span_string(span: Span, bytes: &[u8]) -> Option<String> {
    return match str::from_utf8(&bytes[span.start..span.end]) {
        Ok(s) => Some(String::from(s)),
        Err(_) => None,
    };
}

fn get_word(bytes: &[u8], start: usize) -> Span {
    let mut end = start;
    while end < bytes.len() && !is_delim(bytes[end]){
        end += 1;
    }

    return Span::new(start, end);
}

fn compare(span: &Span, bytes: &[u8], string: &'static str) -> bool {
    if span.end - span.start != string.len() {
        return false;
    }

    let str = String::from(string);
    let slice1 = &bytes[span.start..span.end];
    let slice2= str.as_bytes(); 

    return slice1 == slice2;
}


fn is_delim(b: u8) -> bool {
    return b == b' ' || b == b';' || b == b'\n' 
        || b == b'\t' || b == b'(' || b == b')'
        || b == b'{' || b == b'}';
}

#[derive(Clone, Debug)]
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