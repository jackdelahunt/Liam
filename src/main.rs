use num_derive::FromPrimitive;    
use num_traits::FromPrimitive;

struct VM {
    stack: [u64; 1024],
    memory: [u64; 1024],
    registers: [u64; 10],
    memory_ptr: usize,
    instruction_ptr: usize,
}

impl VM {
    pub fn new() -> Self {
        return Self {stack: [0; 1024], memory: [0; 1024], registers: [0; 10], memory_ptr: 0, instruction_ptr: 0};
    }

    pub fn put(&mut self, value: u64) {
        self.memory[self.memory_ptr] = value as u64;

        self.memory_ptr += 1;
    }

    pub fn run(&mut self) {
        while self.instruction_ptr != self.memory_ptr {
            match FromPrimitive::from_u64(self.memory[self.instruction_ptr]) {
                Some(OP::LOAD) => self.load_op(),
                Some(OP::PRINT) => self.print_op(),
                Some(OP::ADD) => self.add_op(),
                _ => {}
            }

            self.instruction_ptr += 1;
        }
    }

    pub fn load_op(&mut self) {
        let value = self.next();
        let register = self.next();
        self.registers[register as usize] = value;
    }

    pub fn print_op(&mut self) {
        let register = self.next();
        println!("{}", self.registers[register as usize]);
    }

    pub fn add_op(&mut self) {
        let value_1 = self.registers[self.next() as usize];
        let value_2 = self.registers[self.next() as usize];
        let result_register = self.next() as usize;

        self.registers[result_register] = value_1 + value_2;
    }

    fn next(&mut self) -> u64 {
        self.instruction_ptr += 1;
        return self.memory[self.instruction_ptr];
    }
}


#[derive(FromPrimitive)]
enum OP {
    LOAD = 1,
    PRINT = 2,
    ADD = 3,
}

fn main() {
    let mut vm = VM::new();
    vm.put(OP::LOAD as u64);
    vm.put(10);
    vm.put(0);
    vm.put(OP::LOAD as u64);
    vm.put(20);
    vm.put(1);
    vm.put(OP::ADD as u64);
    vm.put(0);
    vm.put(1);
    vm.put(0);
    vm.put(OP::PRINT as u64);
    vm.put(0);

    vm.run();
}
