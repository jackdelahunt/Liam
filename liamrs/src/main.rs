use std::fs;
use std::env;

use liamrs::{Lexer};

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        panic!("Need more command line argumetns...");
    }

    let path = args[1].clone();

    let contents = fs::read_to_string(path)
        .expect("Something went wrong reading the file");

    let lexer = Lexer::new();
    let tokens = lexer.lex(contents);
}
