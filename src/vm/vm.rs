use num_derive::FromPrimitive;    
use num_traits::FromPrimitive;
use crate::vm::Heap;

pub struct VM {
    pub byte_code: [u64; 1024],
    pub stack: [u64; 1024],
    pub heap: Heap,
    pub registers: [u64; 10],
    pub memory_ptr: usize,
    pub stack_ptr: usize,
    pub instruction_ptr: usize,
}

impl VM {
    pub fn new() -> Self {
        return Self {
            byte_code: [0; 1024],
            stack: [0; 1024],
            heap: Heap::new(),
            registers: [0; 10], 
            memory_ptr: 0,
            stack_ptr: 0,
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
                Some(OP::PUSH) => self.push_op(),
                Some(OP::PRINT) => self.print_op(),
                Some(OP::ADD) => self.add_op(),
                Some(OP::STORE) => self.store_op(),
                Some(OP::LOAD) => self.load_op(),
                Some(OP::ALLOC) => self.alloc_op(),
                Some(OP::PUT) => self.put_op(),
                Some(OP::GET) => self.get_op(),
                _ => {}
            }

            self.instruction_ptr += 1;
        }
    }

    fn push_op(&mut self) {
        let value = self.next();
        self.stack_push(value);
    }

    fn print_op(&mut self) {
        println!("{}", self.stack_peek());
    }

    fn add_op(&mut self) {
        let v1 = self.stack_pop();
        let v2 = self.stack_pop();
        self.stack_push(v1 + v2);
    }

    fn store_op(&mut self) {
        let register = self.next() as usize;
        let value = self.stack_pop();
        self.registers[register] = value;
    }

    fn load_op(&mut self) {
        let register = self.next() as usize;
        self.stack_push(self.registers[register]);
    }

    fn alloc_op(&mut self) {
        let size = self.stack_pop();
        let addr = self.heap.alloc(size as usize);
        self.stack_push(addr as u64);
    }

    fn put_op(&mut self) {
        let ptr_register = self.next() as usize;
        let value_register = self.next() as usize;

        self.heap.put(self.registers[ptr_register] as usize, self.registers[value_register])
    }

    fn get_op(&mut self) {
        let ptr_register = self.next() as usize;
        let result_register = self.next() as usize;

        self.registers[result_register] = self.heap.get(self.registers[ptr_register] as usize); 
    }

    fn next(&mut self) -> u64 {
        self.instruction_ptr += 1;
        return self.byte_code[self.instruction_ptr];
    }

    fn stack_push(&mut self, value: u64) {
        self.stack[self.stack_ptr] = value;
        self.stack_ptr += 1;
    }

    fn stack_pop(&mut self) -> u64 {
        self.stack_ptr -= 1;
        return self.stack[self.stack_ptr];
    }

    fn stack_peek(&mut self) -> u64 {
        return self.stack[self.stack_ptr - 1];
    }
}

#[derive(FromPrimitive, Clone, Copy)]
pub enum OP {
    PUSH = 0,
    PRINT = 1,
    ADD = 2,
    STORE = 3,
    LOAD = 4,
    ALLOC = 5,
    PUT = 6,
    GET = 7,
}