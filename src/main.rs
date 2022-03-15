
mod vm;
use crate::vm::{VM, OP};
use crate::vm::generator::{Generator, OPCommand};
use std::fs;

fn main() {

    let contents = fs::read_to_string("/home/jackdelahunt/Projects/rvm/main.x").unwrap();
    
    let mut generator = Generator::new([
        OPCommand::new("push", OP::PUSH, 1),
        OPCommand::new("print", OP::PRINT, 0),
        OPCommand::new("add", OP::ADD, 0),
        OPCommand::new("store", OP::STORE, 1),
        OPCommand::new("load", OP::LOAD, 1),
        OPCommand::new("alloc", OP::ALLOC, 0),
        OPCommand::new("put", OP::PUT, 2),
        OPCommand::new("get", OP::GET, 2),
        ]);
        
        match generator.generate_byte_code(&contents) {
            Ok(_) => {},
            Err(err) => println!("{}", err),
        }
        
    let mut vm = VM::new(generator);
    vm.run();

    // vm.put(OP::LOAD as u64);
    // vm.put(10);
    // vm.put(0);
    // vm.put(OP::LOAD as u64);
    // vm.put(20);
    // vm.put(1);
    // vm.put(OP::ADD as u64);
    // vm.put(0);
    // vm.put(1);
    // vm.put(0);
    // vm.put(OP::PRINT as u64);
    // vm.put(0);

    // vm.run();
}
