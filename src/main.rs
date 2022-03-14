
mod vm;
use crate::vm::VM;
use crate::vm::generator::{generate_byte_code};
use std::fs;

fn main() {

    let mut vm = VM::new();
    let contents = fs::read_to_string("/home/jackdelahunt/Projects/rvm/main.x").unwrap();

    match generate_byte_code(&contents, &mut vm) {
        Ok(_) => {},
        Err(err) => println!("{}", err),
    }

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
