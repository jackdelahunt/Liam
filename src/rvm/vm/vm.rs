use std::isize;

use num_derive::FromPrimitive;    
use num_traits::FromPrimitive;
use crate::vm::Heap;
use crate::vm::generator::Generator;

pub struct VM {
    pub byte_code: [u64; 1024],
    pub stack: [u64; 1024],
    pub heap: Heap,
    pub memory_ptr: usize,
    pub stack_ptr: usize,
    pub instruction_ptr: usize,
    pub frames: Vec<StackFrame>
}

impl VM {
    pub fn new(generator: Generator) -> Self {

        let mut instruction_ptr: Option<usize> = None;

        for i in 0..generator.functions.len() {
            if generator.functions[i].label == "@main" {
                instruction_ptr = Some(generator.functions[i].location);
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
            memory_ptr: generator.memory_ptr,
            stack_ptr: 0,
            instruction_ptr: instruction_ptr.unwrap(),
            frames: vec![StackFrame::new(generator.memory_ptr, 0)] // poping this frame ends program
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
                Some(OP::POPRET) => {
                    // explicity set the ip
                    self.pop_return_op();
                }
                Some(OP::CALL) => {
                    // explicity set the ip
                    self.call_op();
                }
                Some(OP::GOTO) => {
                    // explicity set the ip
                    self.goto_op();
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
        if self.stack_ptr >= self.stack.len() {
            panic!("Stack overflow")
        }

        self.stack[self.stack_ptr] = value;
        self.stack_ptr += 1;
    }

    fn stack_pop(&mut self) -> u64 {
        self.stack_ptr = self.from_stack_ptr(-1);
        return self.stack[self.stack_ptr];
    }

    fn stack_peek(&mut self) -> u64 {
        return self.stack[self.from_stack_ptr(-1)];
    }

    fn from_stack_ptr(&mut self, offset: isize) -> usize {
        let result = self.stack_ptr.change_signed(offset).expect("Cannot pop off stack, stack is empty");
        if result < self.current_frame().stack_ptr {
            panic!("Out of bounds stack ptr");
        }

        return result;
    }

    fn current_frame(&mut self) -> &mut StackFrame {
        return match self.frames.last_mut() {
            Some(frame) => frame,
            None => panic!("Out of stack frames"),
        }
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
        self.current_frame().locals[register] = value;
    }

    fn load_op(&mut self) {
        let register = self.next() as usize;
        let local_value = self.current_frame().locals[register];
        self.stack_push(local_value);
    }

    fn alloc_op(&mut self) {
        let size = self.stack_pop();
        let addr = self.heap.alloc(size as usize);
        self.stack_push(addr as u64);
    }

    fn put_op(&mut self) {
        let value = self.stack_pop();
        let ptr = self.stack_pop() as usize;

        self.heap.put(ptr, value);
    }

    fn get_op(&mut self) {
        let ptr = self.stack_pop() as usize;

        let heap_value = self.heap.get(ptr);
        self.stack_push(heap_value); 
    }

    fn return_op(&mut self) {
        let popped_frame = self.frames.pop().unwrap();
        self.instruction_ptr = popped_frame.return_address;
        self.stack_ptr = popped_frame.stack_ptr;
    }

    fn pop_return_op(&mut self) {
        let returning_value = self.stack_pop();
        let popped_frame = self.frames.pop().unwrap();
        self.instruction_ptr = popped_frame.return_address;
        self.stack_ptr = popped_frame.stack_ptr;
        self.stack_push(returning_value);
    }

    fn call_op(&mut self) {
        let callee = self.next() as usize;
        let arg_count = self.next() as usize;

        let mut frame = StackFrame::new(self.instruction_ptr + 1, self.stack_ptr - arg_count);

        for i in (0..arg_count).rev() {
            frame.locals[i] = self.stack_pop();
        }

        self.frames.push(frame);

        self.instruction_ptr = callee;
    }

    fn goto_op(&mut self) {
        let callee = self.next() as usize;
        self.instruction_ptr = callee;
    }
}

pub struct StackFrame {
    pub return_address: usize,  // if returned where will the ip go to
    pub stack_ptr: usize,       // if returned where will the sp go
    pub locals: [u64; 32],      // local variables
}

impl StackFrame {
    pub fn new(return_address: usize, stack_ptr: usize) -> Self {
        Self {
            return_address, 
            stack_ptr,
            locals: [0; 32],
        }
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
    POPRET  = 10,
    CALL    = 11,
    GOTO    = 12,
}

pub trait SignedChanged {
    fn change_signed(&self, offset: isize) -> Option<Self> where Self: Sized;
}

impl SignedChanged for usize {
    fn change_signed(&self, offset: isize) -> Option<Self> {
        if offset >= 0 {
            return self.checked_add(offset as usize);
        } else {    
            let new_value = *self as isize + offset;
            if new_value < 0 {
                return None
            }

            return Some(new_value as usize);
        }
    }
}