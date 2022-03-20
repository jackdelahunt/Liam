mod vm;
use crate::vm::generator::{Generator, OPCommand};
use crate::vm::{OP, VM};
use std::fs;

fn main() {
    let contents = fs::read_to_string("/home/jackdelahunt/Projects/rvm/main.liam").unwrap();

    let mut generator = Generator::new([
        OPCommand::new("push", OP::PUSH, 1),
        OPCommand::new("pop", OP::POP, 0),
        OPCommand::new("print", OP::PRINT, 0),
        OPCommand::new("add", OP::ADD, 0),
        OPCommand::new("store", OP::STORE, 1),
        OPCommand::new("load", OP::LOAD, 1),
        OPCommand::new("alloc", OP::ALLOC, 0),
        OPCommand::new("put", OP::PUT, 0),
        OPCommand::new("get", OP::GET, 0),
        OPCommand::new("ret", OP::RET, 0),
        OPCommand::new("pop_ret", OP::POPRET, 0),
        OPCommand::new("call", OP::CALL, 2),
        OPCommand::new("goto", OP::GOTO, 1),
    ]);

    match generator.generate_byte_code(&contents) {
        Ok(_) => {}
        Err(err) => println!("{}", err),
    }

    let mut vm = VM::new(generator);
    vm.run();
}
