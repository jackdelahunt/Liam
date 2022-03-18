
pub struct Program {
    pub statements: Vec<Statement>
}

impl Program {
    pub fn new(statements: Vec<Statement>) -> Self {
        return Self {statements};
    }
}

pub enum Statement {
    Print{literal: i32}
}