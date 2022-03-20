mod ast;

mod compiler;
use compiler::Compiler;

use std::fs::File;
use std::io::prelude::*;

#[macro_use] extern crate lalrpop_util;
lalrpop_mod!(pub parser, "/liamc/parser.rs"); // synthesized by LALRPOP


fn main() {
    let program = parser::ProgramParser::new()
        .parse("print 10")
        .unwrap();

    let mut compiler = Compiler::new();
    compiler.compile(program);

    let mut file = File::create("main.liam").unwrap();
    file.write_all(compiler.generted_code.as_bytes()).unwrap();
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

#[test]
fn binary_expressions() {
    _ = parser::BinaryExpressionParser::new()
        .parse("10 * (10 * 10)")
        .unwrap();
}