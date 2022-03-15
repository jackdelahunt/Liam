use num_derive::FromPrimitive;    
use num_traits::FromPrimitive;
use crate::vm::Heap;

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
                Some(OP::MOV) => self.move_op(),
                Some(OP::PRINT) => self.print_op(),
                Some(OP::ADD) => self.add_op(),
                Some(OP::ALLOC) => self.alloc_op(),
                Some(OP::PUT) => self.put_op(),
                Some(OP::GET) => self.get_op(),
                _ => {}
            }

            self.instruction_ptr += 1;
        }
    }

    fn move_op(&mut self) {
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
}

#[derive(FromPrimitive, Clone, Copy)]
pub enum OP {
    MOV = 0,    // move value into a register
    PRINT = 1,  // print value
    ADD = 2,    // add to register values
    ALLOC = 3,  // allocate memory on the heap
    PUT = 4,    // set a value on the heap
    GET = 5,    // get a value on the heap
}