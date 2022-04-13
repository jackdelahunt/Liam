pub struct Heap {
    pub memory: Vec<u64>,
}

impl Heap {
    pub fn new() -> Self {
        return Heap { memory: Vec::new() };
    }

    pub fn alloc(&mut self, size: usize) -> usize {
        let ptr = self.memory.len();
        self.memory.resize(ptr + size, 0);
        return ptr;
    }

    pub fn put(&mut self, address: usize, value: u64) {
        self.memory[address] = value;
    }

    pub fn get(&mut self, address: usize) -> u64 {
        return self.memory[address];
    }
}
