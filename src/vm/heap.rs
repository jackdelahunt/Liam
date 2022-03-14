pub struct Heap {
    memory: Vec<u64>
}

impl Heap {
    pub fn new() -> Self {
        return Heap{
            memory: Vec::new()
        };
    }

    pub fn alloc(&mut self, size: usize) -> u64 {
        let ptr = self.memory.len();
        self.memory.resize(ptr + size, 0);
        return ptr as u64;
    }
}