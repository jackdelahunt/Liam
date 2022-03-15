use crate::vm::OP;
use crate::vm::VM;

struct OPCommand {
    name: &'static str,
    op: OP,
    arguments: u32
}

static OPCommands: [OPCommand; 4] = [
    OPCommand{name: "mov", op: OP::MOV, arguments: 2},
    OPCommand{name: "print", op: OP::PRINT, arguments: 1},
    OPCommand{name: "add", op: OP::ADD, arguments: 3},
    OPCommand{name: "alloc", op: OP::ALLOC, arguments: 2},
];

pub fn generate_byte_code(code: &String, vm: &mut VM) -> Result<(), String> {

    let mut split = code.split_whitespace();
    loop {
        match split.next() {
            Some(word) => {
                match get_command_index(word) {
                    Some(index) => {
                        vm.put(OPCommands[index].op as u64);
                        for _ in 0..OPCommands[index].arguments {
                            match split.next() {
                                Some(word) => {
                                    let value = match word.parse::<u64>() {
                                        Ok(n) => n,
                                        Err(_) => return Err(format!("Cannot convert '{value}' to number", value=word)),
                                    };

                                    vm.put(value);
                                },
                                None => return Err(format!("Not enough arguments for {name}", name=OPCommands[index].name)),
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

fn get_command_index(word: &str) -> Option<usize> {
    for command_index in 0..OPCommands.len() {
        if OPCommands[command_index].name == word {
            return Some(command_index);
        }
    }

    return None;
}

#[cfg(test)]
mod tests {
    use crate::vm::{VM, generator::generate_byte_code, OP};


    #[test]
    fn single_command() {
        let mut vm = VM::new();
        let code = String::from("mov 1 2");
        generate_byte_code(&code, &mut vm).unwrap();

        assert_eq!(vm.byte_code[0], OP::MOV as u64);
        assert_eq!(vm.byte_code[1], 1);
        assert_eq!(vm.byte_code[2], 2);
    }

    #[test]
    fn multiple_commands() {
        let mut vm = VM::new();
        let code = String::from(
            "add 1 2 3\n
            print 2"
        );

        generate_byte_code(&code, &mut vm).unwrap();

        assert_eq!(vm.byte_code[0], OP::ADD as u64);
        assert_eq!(vm.byte_code[1], 1);
        assert_eq!(vm.byte_code[2], 2);
        assert_eq!(vm.byte_code[3], 3);
        assert_eq!(vm.byte_code[4], OP::PRINT as u64);
        assert_eq!(vm.byte_code[5], 2);
    }
}