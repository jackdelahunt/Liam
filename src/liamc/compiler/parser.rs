use crate::compiler::Token;

use super::TokenType;

pub struct Parser {
    tokens: Vec<Token>,
    pub root: Node
}

impl Parser {
    pub fn new(tokens: Vec<Token>) -> Self {
        return Self{tokens, root: Node::new(NodeType::File, String::new())};
    }

    pub fn parse(&mut self) -> Result<(), String> {
        let expression = self.parse_expression()?;
        self.root.nodes.push(expression);
        return Ok(());
    }

    fn parse_expression(&mut self) -> Result<Node, String> {
        let int_1 = self.tokens.pop().unwrap();
        let op = self.tokens.pop().unwrap();
        let int_2 = self.tokens.pop().unwrap();

        let mut node = Node::new(NodeType::BinaryExpression, String::new());
        node.nodes.push(Node::create_from(int_1));
        node.nodes.push(Node::create_from(op));
        node.nodes.push(Node::create_from(int_2));

        return Ok(node);
    }
}

#[derive(Debug)]
pub enum NodeType {
    File,
    // expressions
    BinaryExpression,
    IntLiteral,

    // base nodes 
    Identifier,
    Plus
}

#[derive(Debug)]
pub struct Node {
    node_type: NodeType,
    nodes: Vec<Node>,
    string: String
}

impl Node {
    pub fn new(node_type: NodeType, string: String) -> Self {
        return Node{node_type, nodes: Vec::new(), string};
    }

    pub fn create_from(token: Token) -> Self {
        match token.token_type {
            TokenType::IntLiteral => {
                Node::new(NodeType::IntLiteral, token.string)
            },
            TokenType::Identifier => {
                Node::new(NodeType::IntLiteral, token.string)
            },
            TokenType::Plus => {
                Node::new(NodeType::Plus, token.string)
            },
            _ => panic!("Cannot create node from this token")
        }
    }
}