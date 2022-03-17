use crate::vm::{OP};

pub struct OPCommand {
    name: &'static str,
    op: OP,
    arguments: u32
}

impl OPCommand {
    pub fn new(name: &'static str, op: OP, arguments: u32) -> Self {
        return Self {name, op, arguments};
    }
}

pub struct Generator {
    pub instruction_set: [OPCommand; 11],
    pub byte_code: [u64; 1024],
    pub memory_ptr: usize,
    pub labels: Vec<Label>,
}

impl Generator {
    pub fn new(instruction_set: [OPCommand; 11]) -> Self {
        Self {
            instruction_set,
            byte_code: [0; 1024],
            memory_ptr: 0,
            labels: Vec::new()
        }
    }

    pub fn generate_byte_code(&mut self, code: &String) -> Result<(), String> {

        let mut split = code.split_whitespace();
        loop {
            match split.next() {
                Some(word) => {

                    // check if this is a label
                    if word.starts_with("@") {
                        self.labels.push(Label::new(String::from(word), self.memory_ptr));
                        continue;
                    }

                    match self.get_instruction_index(word) {
                        Some(index) => {
                            self.put(self.instruction_set[index].op as u64);
                            for _ in 0..self.instruction_set[index].arguments {
                                match split.next() {
                                    Some(word) => {

                                        let value: u64;
                                        // check if this argument is looking for a label
                                        if word.starts_with("@") {
                                            value = match self.get_label_location(word) {
                                                Some(n) => n as u64,
                                                None => return Err(format!("Cannot find label '{label}'", label=word)),
                                            }
                                        } else {
                                            value = match word.parse::<u64>() {
                                                Ok(n) => n,
                                                Err(_) => return Err(format!("Cannot convert '{value}' to number", value=word)),
                                            };
                                        }
    
                                        self.put(value);
                                    },
                                    None => return Err(format!("Not enough arguments for {name}", name=self.instruction_set[index].name)),
                                }
                            }
                        },
                        None => return Err(format!("No command found called `{word}`", word=word)),
                    }
                },
                None => break
            }
        }
    
        return Ok(());
    }

    fn get_label_location(&self, label: &str) -> Option<usize> {
        for i in 0..self.labels.len() {
            if self.labels[i].label == label {
                return Some(self.labels[i].location);
            }
        }

        return None;
    }

    fn put(&mut self, value: u64) {
        self.byte_code[self.memory_ptr] = value as u64;

        self.memory_ptr += 1;
    }

    fn get_instruction_index(&self, word: &str) -> Option<usize> {
        for command_index in 0..self.instruction_set.len() {
            if self.instruction_set[command_index].name == word {
                return Some(command_index);
            }
        }
    
        return None;
    }
}

pub struct Label {
    pub label: String,
    pub location: usize
}

impl Label {
    pub fn new(label: String, location: usize) -> Self {
        Self {label, location}
    }
}