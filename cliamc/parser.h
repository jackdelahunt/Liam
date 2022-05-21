#pragma once

#include "str.h"
#include "lexer.h"

#define SUB_NODE_SIZE 128

typedef enum NodeType NodeType;
typedef struct FileNode FileNode;
typedef struct AssignmentNode AssignmentNode;
typedef struct FnDeclNode FnDeclNode;
typedef struct  IdenExprNode IdenExprNode;
typedef struct IntExprNode IntExprNode;
typedef struct BinaryExprNode BinaryExprNode;
typedef struct AstNode AstNode;
typedef struct Parser Parser;

typedef enum NodeType {
    NODE_FILE,
    NODE_ASSIGNMENT,
    NODE_FN_DECL,
    NODE_IDEN_EXPR,
    NODE_INT_EXPR,
    NODE_SCOPE,
    NODE_BINARY_EXPR,
} NodeType;

typedef struct FileNode {
    AstNode* sub_nodes[SUB_NODE_SIZE];
} FileNode;

typedef struct AssignmentNode {
    Slice identifier;
    AstNode* assigned_to;
} AssignmentNode;

typedef struct FnDeclNode {
    Slice identifier;
    AstNode* body;
} FnDeclNode;

typedef struct ScopeNode {
    AstNode* sub_nodes[SUB_NODE_SIZE];
} ScopeNode;

typedef struct IdenExprNode {
    Slice identifier;
} IdenExprNode;

typedef struct IntExprNode {
    Slice literal;
} IntExprNode;

typedef struct BinaryExprNode {
    AstNode* left;
    enum OpType {
        OP_ADD,
        OP_MULT,
        OP_OR,
        OP_AND,
        OP_EQUAL,
    } OpType;
    AstNode* right;
} BinaryExprNode;

typedef struct AstNode {
    NodeType node_type;
    union {
        FnDeclNode fn_node;
        AssignmentNode assignment_node;
        FileNode file_node;
        IdenExprNode iden_expr_node;
        IntExprNode int_expr_node;
        ScopeNode scope_node;
        BinaryExprNode binary_expr_node;
    } data;
} AstNode;

typedef struct Parser {
    Token* tokens;
    size_t count;
    size_t current_token;
    AstNode* root;
} Parser;

AstNode* make_node(NodeType type);
Parser make_parser(Lexer* lexer);
AstNode* make_ast(Parser* parser);
AstNode* make_assign(Parser* parser);
AstNode* make_fn(Parser* parser);
AstNode* make_scope(Parser* parser);
AstNode* make_expr_stmt(Parser* parser);
AstNode* make_expr(Parser* parser);
AstNode* make_or(Parser* parser);
AstNode* make_and(Parser* parser);
AstNode* make_compare(Parser* parser);
AstNode* make_term(Parser* parser);
AstNode* make_factor(Parser* parser);
AstNode* make_postfix(Parser* parser);
AstNode* make_call(Parser* parser);
AstNode* make_array(Parser* parser);
AstNode* make_primary(Parser* parser);
AstNode* make_new(Parser* parser);
AstNode* make_group(Parser* parser);
Token* peek(Parser* parser);
Token* consume_token(Parser* parser, TokenType type);
void print_ast(Parser* parser);
