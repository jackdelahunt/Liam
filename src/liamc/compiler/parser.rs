use crate::compiler::Token;

use super::TokenType;

pub struct Parser<'a> {
    tokens: Vec<Token>,
    pub root: Node<'a>,
    source: String
}

impl<'a> Parser<'a> {
    pub fn new(tokens: Vec<Token>, source: String) -> Self {
        let root = Node::new(NodeType::File, &source.as_str(), 0, source.len());
        
        return Self {
            tokens, 
            root, 
            source
        };
    }

    pub fn parse(&mut self) -> Result<(), String> {
        // let expression = self.parse_expression()?;
        // self.root.nodes.push(expression);
        return Ok(());
    }

    fn parse_binary_expression(&mut self) -> Result<Node, String> {
        let int_1 = self.parse_expression()?;
        let op = self.parse_expression()?;
        let int_2 = self.parse_expression()?;

        let mut node = Node::new(NodeType::BinaryExpression, &self.source[int_1.start..int_2.end], int_1.start, int_2.end);
        node.nodes.push(int_1);
        node.nodes.push(op);
        node.nodes.push(int_2);
        return Ok(node);
    }

    fn parse_expression(&mut self) -> Result<Node, String> {
        let token = self.tokens.pop();
        match token {
            Some(t) => {
                Err(String::from("No more tokens to parse"))
            },
            None => Err(String::from("No more tokens to parse")),
        }   
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
pub struct Node<'a> {
    node_type: NodeType,
    nodes: Vec<Node<'a>>,
    string: &'a str,
    start: usize,
    end: usize,
}

impl<'a> Node<'a> {
    pub fn new(node_type: NodeType, string: &'a str, start: usize, end: usize) -> Self {
        return Node{node_type, nodes: Vec::new(), string, start, end};
    }
}