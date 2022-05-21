#pragma once

#include "str.h"
#include "lexer.h"

#define FILE_NODE_SIZE 128

typedef enum NodeType NodeType;
typedef struct FileNode FileNode;
typedef struct AssignmentNode AssignmentNode;
typedef struct FnDeclNode FnDeclNode;
typedef struct AstNode AstNode;
typedef struct Parser Parser;

typedef enum NodeType {
    NODE_FILE,
    NODE_ASSIGNMENT,
    NODE_FN_DECL
} NodeType;

typedef struct FileNode {
    AstNode* sub_nodes[FILE_NODE_SIZE];
} FileNode;

typedef struct AssignmentNode {
    Slice identifier;
    AstNode* assigned_to;
} AssignmentNode;

typedef struct FnDeclNode {
    Slice identifier;
} FnDeclNode;


typedef struct AstNode {
    NodeType node_type;
    union {
        FnDeclNode fn_node;
        AssignmentNode assignment_node;
        FileNode file_node;
    } data;
} AstNode;

typedef struct Parser {
    Token* tokens;
    size_t count;
    size_t current_token;
    AstNode* root;
} Parser;

AstNode* make_node_of_type(NodeType type);
Parser make_parser(Lexer* lexer);
AstNode* make_ast(Parser* parser);
AstNode* make_let(Parser* parser);
AstNode* make_fn(Parser* parser);
Token* peek(Parser* parser);
Token* consume_token(Parser* parser, TokenType type);
void print_ast(Parser* parser);
