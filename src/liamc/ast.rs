
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

pub enum Expression {
    BinaryExpression{lhs: Box<Expression>, op: Operator, rhs: Box<Expression>},
    LiteralExpression{literal: i32},
}

pub enum Operator {
    Add,
    Minus,
    Mult,
    Divide
}