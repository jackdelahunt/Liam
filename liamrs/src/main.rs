use std::fs;
use std::env;

use liamrs;

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        panic!("Need more command line argumetns...");
    }

    let path = args[1].clone();

    let contents = fs::read_to_string(path)
        .expect("Something went wrong reading the file");

    let tokens = liamrs::lex(contents);
    for t in &tokens {
        println!("{:?}", t);
    }
}
