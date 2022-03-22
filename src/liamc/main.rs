extern crate pest;
#[macro_use]
extern crate pest_derive;

use pest::{Parser, iterators::{Pairs, Pair}};
use std::{fs, collections::HashMap};
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
    generated_code: String,
    context: Context
}

impl Compiler {
    fn new() -> Self {
        let mut s =  Self {
            generated_code: String::new(),
            context: Context::new()
        };

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
            Rule::let_statement => return Some(self.eval_let_statement(pair)),
            Rule::EOI => None,
            _ => panic!("Could not eval this pair {:?}", pair)
        }
    }

    fn eval_expression(&mut self, pair: Pair<Rule>) -> String {
        match pair.as_rule() {
            Rule::binary_expression => return self.eval_binary_expression(pair),
            Rule::identifier => return self.eval_identifier(pair),
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

    fn eval_identifier(&mut self, identifier: Pair<Rule>) -> String {
        let identifier_name = String::from( identifier.as_str());
        let register = self.context.variables.get(&identifier_name).expect(format!("No identifier with name {}", identifier_name).as_str());
        

        return format!("load {}\n", register);
    }

    fn eval_print_statement(&mut self) -> String {
        return String::from("print\n");
    }

    fn eval_let_statement(&mut self, let_statement: Pair<Rule>) -> String {
        let mut sub_pairs = let_statement.into_inner();

        let identifier = sub_pairs.next().expect("No identifier in this let statement");
        let register = self.context.declare_variable(String::from( identifier.as_str()));

        // We know this is always a binary expression for now
        let expression = sub_pairs.next().expect("No expression in this let statement");

        let evaled_expression = self.eval_expression(expression);
        return format!("{}store {}\n", evaled_expression, register);
    }
}

struct Context {
    variables: HashMap<String, u64>,
    free_variable_register: u64
}

impl Context {
    fn new() -> Self {
        return Self {
            variables: HashMap::new(),
            free_variable_register: 0
        };
    }

    fn declare_variable(&mut self, identifier: String) -> u64 {
        let register = self.free_variable_register;
        self.variables.insert(identifier, register);
        self.free_variable_register += 1;
        return register;

    }
}

