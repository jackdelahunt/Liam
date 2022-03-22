extern crate pest;
#[macro_use]
extern crate pest_derive;

use pest::{Parser, iterators::{Pairs, Pair}};
use std::fs;
use std::fs::File;
use std::io::prelude::*;

#[derive(Parser)]
#[grammar = "/home/jackdelahunt/Projects/Liam/src/liamc/parser.pest"]
pub struct FileParser;

fn main() {
    let source = fs::read_to_string("main.liam")
        .expect("Something went wrong reading the file");


    let file = FileParser::parse(Rule::file, source.as_str()).unwrap();
    let mut compiler = Compiler::new();
    compiler.compile(file);

    let mut file = File::create("main.l__m").unwrap();
    file.write_all(compiler.generated_code.as_bytes()).unwrap();
}

// fn print_type_of<T>(_: &T) {
//     println!("{}", std::any::type_name::<T>())
// }

struct Compiler {
    generated_code: String
}

impl Compiler {
    fn new() -> Self {
        let mut s =  Self {generated_code: String::new()};
        s.generated_code.push_str("@main\n");
        return s;
    }

    fn compile(&mut self, mut pairs: Pairs<Rule>) {
        let file = pairs.next().unwrap();
        if file.as_rule() != Rule::file {
            panic!("Did not receive file to compile");
        }

        for pair in file.into_inner() {
            if let Some(new_source) = self.eval_statement(pair) {
                self.generated_code += new_source.as_str();
            }
        }
    }

    fn eval_statement(&mut self, pair: Pair<Rule>) -> Option<String> {
        match pair.as_rule() {
            Rule::binary_expression => return Some(self.eval_binary_expression(pair)),
            Rule::print_statement => return Some(self.eval_print_statement()),
            Rule::EOI => None,
            _ => panic!("Could not eval this pair {:?}", pair)
        }
    }

    fn eval_binary_expression(&mut self, binary_expression: Pair<Rule>) -> String {
        let mut pairs = binary_expression.into_inner();

        let lhs = pairs.next().unwrap();
        let _ = pairs.next().unwrap();
        let rhs = pairs.next().unwrap();

        let mut output = String::new();

        output.push_str("push ");
        output.push_str(lhs.as_str());
        output.push_str("\n");

        output.push_str("push ");
        output.push_str(rhs.as_str());
        output.push_str("\n");

        output.push_str("add\n");

        return output;
    }

    fn eval_print_statement(&mut self) -> String {
        return String::from("print\n");
    }
}

