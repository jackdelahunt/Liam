use num_derive::FromPrimitive;    
use num_traits::FromPrimitive;
use crate::vm::Heap;
use std::fs;

pub struct VM {
    pub byte_code: [u64; 1024],
    pub heap: Heap,
    pub registers: [u64; 10],
    pub memory_ptr: usize,
    pub instruction_ptr: usize,
}

impl VM {
    pub fn new() -> Self {
        return Self {
            byte_code: [0; 1024],
            heap: Heap::new(),
            registers: [0; 10], 
            memory_ptr: 0, 
            instruction_ptr: 0
        };
    }

    pub fn put(&mut self, value: u64) {
        self.byte_code[self.memory_ptr] = value as u64;

        self.memory_ptr += 1;
    }

    pub fn run(&mut self) {
        while self.instruction_ptr != self.memory_ptr {
            match FromPrimitive::from_u64(self.byte_code[self.instruction_ptr]) {
                Some(OP::LOAD) => self.load_op(),
                Some(OP::PRINT) => self.print_op(),
                Some(OP::ADD) => self.add_op(),
                Some(OP::ALLOC) => self.alloc_op(),
                _ => {}
            }

            self.instruction_ptr += 1;
        }
    }

    fn load_op(&mut self) {
        let value = self.next();
        let register = self.next();
        self.registers[register as usize] = value;
    }

    fn print_op(&mut self) {
        let register = self.next();
        println!("{}", self.registers[register as usize]);
    }

    fn add_op(&mut self) {
        let value_1 = self.registers[self.next() as usize];
        let value_2 = self.registers[self.next() as usize];
        let result_register = self.next() as usize;

        self.registers[result_register] = value_1 + value_2;
    }

    fn alloc_op(&mut self) {
        let size = self.next() as usize;
        let result_register = self.next() as usize;

        self.registers[result_register] = self.heap.alloc(size);
    }

    fn next(&mut self) -> u64 {
        self.instruction_ptr += 1;
        return self.byte_code[self.instruction_ptr];
    }
}

#[derive(FromPrimitive, Clone, Copy)]
pub enum OP {
    LOAD = 0,
    PRINT = 1,
    ADD = 2,
    ALLOC = 3,
}