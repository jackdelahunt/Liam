use crate::ast::{Program, Statement};

pub struct Compiler {
    pub generted_code: String,
}

impl Compiler {
    pub fn new() -> Self {
        return Self {
            generted_code: String::new()
        };
    }

    pub fn compile(&mut self, program: Program) {
        self.generted_code.push_str("@main\n");
        self.eval_program(program);
    }

    fn eval_program(&mut self, program: Program) {
        for statement in program.statements {
            self.eval_statement(statement);
        }
    }
    
    fn eval_statement(&mut self, statement: Statement) {
        match statement {
            Statement::Print { literal } => self.eval_print_statement(literal),
        }
    }
    
    fn eval_print_statement(&mut self, literal: i32) {
        self.generted_code.push_str((String::from("push ") + literal.to_string().as_str() + "\n").as_str());
        self.generted_code.push_str("print");
    }
}