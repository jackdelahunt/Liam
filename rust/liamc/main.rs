mod compiler;
use compiler::{Lexer, Parser};

use std::{fs};

fn main() {
    let source = fs::read_to_string("main.liam")
        .expect("Something went wrong reading the file");

    let mut lexer = Lexer::new(source.clone());
    
    lexer.lex();
    // for token in &lexer.tokens {
    //     println!("{:?}", token);
    // }

    let mut parser = Parser::new(lexer.tokens, source.clone());
    _ = parser.parse();
    println!("{:#?}", parser.root);
    return;
}