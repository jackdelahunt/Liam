use ast::{Program, Statement};

#[macro_use] extern crate lalrpop_util;

lalrpop_mod!(pub parser, "/liamc/parser.rs"); // synthesized by LALRPOP
mod ast;

fn main() {
    let program = parser::ProgramParser::new()
        .parse("print 10 print 10")
        .unwrap();

    eval_program(program);
}

fn eval_program(program: Program) {
    for statement in program.statements {
        eval_statement(statement);
    }
}

fn eval_statement(statement: Statement) {
    match statement {
        Statement::Print { literal } => eval_print_statement(literal),
    }
}

fn eval_print_statement(literal: i32) {
    print!("{}", literal);
}

#[test]
fn print() {
    _ = parser::StatementParser::new()
        .parse("print 10")
        .unwrap();
}

#[test]
fn programs() {
    _ = parser::ProgramParser::new()
        .parse("print 10 print 10")
        .unwrap();

}