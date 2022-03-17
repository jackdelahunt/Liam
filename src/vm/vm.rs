use num_derive::FromPrimitive;    
use num_traits::FromPrimitive;
use crate::vm::Heap;
use crate::vm::generator::Generator;

pub struct VM {
    pub byte_code: [u64; 1024],
    pub stack: [u64; 1024],
    pub heap: Heap,
    pub registers: [u64; 10],
    pub memory_ptr: usize,
    pub stack_ptr: usize,
    pub instruction_ptr: usize,
    pub frames: Vec<StackFrame>
}

impl VM {
    pub fn new(generator: Generator) -> Self {

        let mut instruction_ptr: Option<usize> = None;

        for i in 0..generator.labels.len() {
            if generator.labels[i].label == "@main" {
                instruction_ptr = Some(generator.labels[i].location);
                break;
            }
        }

        if instruction_ptr == None {
            panic!("No main label found");
        }

        return Self {
            byte_code: generator.byte_code,
            stack: [0; 1024],
            heap: Heap::new(),
            registers: [0; 10], 
            memory_ptr: generator.memory_ptr,
            stack_ptr: 0,
            instruction_ptr: instruction_ptr.unwrap(),
            frames: vec![StackFrame::new(generator.memory_ptr)] // poping this frame ends program
        };
    }

    pub fn run(&mut self) {
        while self.instruction_ptr < self.memory_ptr {
            match FromPrimitive::from_u64(self.byte_code[self.instruction_ptr]) {
                Some(OP::PUSH) => {
                    self.push_op();
                    self.instruction_ptr += 1;
                }
                Some(OP::POP) => {
                    self.pop_op();
                    self.instruction_ptr += 1;
                }
                Some(OP::PRINT) => {
                    self.print_op();
                    self.instruction_ptr += 1;
                }
                Some(OP::ADD) => {
                    self.add_op();
                    self.instruction_ptr += 1;
                }
                Some(OP::STORE) => {
                    self.store_op();
                    self.instruction_ptr += 1;
                }
                Some(OP::LOAD) => {
                    self.load_op();
                    self.instruction_ptr += 1;
                }
                Some(OP::ALLOC) => {
                    self.alloc_op();
                    self.instruction_ptr += 1;
                }
                Some(OP::PUT) => {
                    self.put_op();
                    self.instruction_ptr += 1;
                }
                Some(OP::GET) => {
                    self.get_op();
                    self.instruction_ptr += 1;
                }
                Some(OP::RET) => {
                    // explicity set the ip
                    self.return_op();
                }
                Some(OP::CALL) => {
                    // explicity set the ip
                    self.call_op();
                }
                _ => {}
            }
        }
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

    fn push_op(&mut self) {
        let value = self.next();
        self.stack_push(value);
    }

    fn pop_op(&mut self) {
        self.stack_pop();
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

    fn return_op(&mut self) {
        let popped_frame = self.frames.pop().unwrap();
        self.instruction_ptr = popped_frame.return_address;
    }

    fn call_op(&mut self) {
        let callee = self.next() as usize;
        self.frames.push(StackFrame::new(self.instruction_ptr + 1));

        self.instruction_ptr = callee;
    }
}

pub struct StackFrame {
    pub return_address: usize,
}

impl StackFrame {
    pub fn new(return_address: usize) -> Self {
        Self {return_address}
    }
}

#[derive(FromPrimitive, Clone, Copy)]
pub enum OP {
    PUSH    = 0,
    POP     = 1,
    PRINT   = 2,
    ADD     = 3,
    STORE   = 4,
    LOAD    = 5,
    ALLOC   = 6,
    PUT     = 7,
    GET     = 8,
    RET     = 9,
    CALL    = 10,
}