#include "str.h"
#include "parser.h"
#include "arena.h"
#include "lexer.h"

/*
 *  --- str.h
 */
Slice make_slice(const char *start, size_t length) {
    return (Slice){start, length};
}

Slice make_slice_c_str(const char *start) {
    size_t length = strlen(start);
    return (Slice){start, length};
}

bool compare_slice(Slice *l, const char *c_str) {
    size_t length = strlen(c_str);

    if(l->length != length) return false;

    for(int i = 0; i < l->length; i++) {
        if(l->start[i] != c_str[i]) return false;
    }

    return true;
}

void print_slice(Slice *slice) {
    for(int i = 0; i < slice->length; i++) {
        printf("%c", slice->start[i]);
    }
}

bool is_char_num(char c) {
    // 48 is zero... 57 is 9
    return (int)c >= 48 && (int)c <= 57;
}

bool slice_to_int(Slice *slice, int *dest) {
    const char* current = slice->start;
    int factor = slice->length - 1;
    int final_value = 0;

    for(int i = 0; i < slice->length; i++) {

        if(is_char_num(*current)) {
            int converted = *current - 48; // remove 0 base number to map 0 -> 0 and so on
            int multiplier = (int)pow(10, factor);
            final_value += converted * multiplier;
        } else {return false; };

        factor--;
        current += 1;
    }

    *dest = final_value;
    return true;
}

Str make_string(const char *c_str) {
    size_t length = strlen(c_str);
    size_t capacity = (length * 2) + 10;
    char* buffer = ALLOC(capacity);
    strncpy(buffer, c_str, length);
    return (Str){buffer, length, capacity};
}

Str make_big_string(const char *c_str) {
    size_t length = strlen(c_str);
    size_t capacity = 2048 + length;
    char* buffer = ALLOC(capacity);
    strncpy(buffer, c_str, length);
    return (Str){buffer, length, capacity};
}

void append_string(Str *str, char c) {
    if(str->length >= str->capacity) {
        str->capacity *= 2;
        GROW(str->buffer, str->capacity);
    }

    str->buffer[str->length++] = c;
}

char char_at_string(Str *str, size_t index) {
    assert(str->length > index);
    return str->buffer[index];
}

void concat_string(Str *dest, Str *src) {
    if(dest->capacity - dest->length < src->length) {
        dest->capacity = (dest->capacity * 2) + src->length;
        GROW(dest->buffer, dest->capacity);
    }

    strcpy(&dest->buffer[dest->length], src->buffer);
    dest->length += src->length;
}

Str read_file(const char *path) {
    Str source = make_big_string("");
    FILE *file = fopen(path, "r");

    int c = fgetc(file);
    while (c != EOF) {
        append_string(&source, (char)c);
        c = fgetc(file);
    }

    fclose(file);
    return source;
}

/*
 * --- arena.h
 */

Arena* current__working__arena__ = NULL;

Arena make_arena(size_t size) {
    return (Arena){malloc(size), size, 0};
}

void* alloc_arena(Arena* arena, size_t size) {
    ASSERT(current__working__arena__, "No arena currently set");
    void* ptr = arena->buffer + arena->base;
    arena->base += size;
    assert(arena->base < arena->size);
    return ptr;
}

void free_arena(Arena* arena) {
    free(arena->buffer);
    arena->buffer = NULL;
    if(current__working__arena__ == arena) {
        current__working__arena__ = NULL;
    }
}

/*
 * --- lexer.h
 */
Token make_token(Slice slice, int type) {
    return (Token){slice, type};
}

bool is_delim(char c) {
    return c == ' ' || c == '\n' || c == EOF || c == '\0' || c == ';' || c == '"' || c == '(' || c == ')';
}

Lexer make_lexer() {
    return (Lexer){NULL, 0};
}

Slice get_word(char* start) {
    size_t index = 0;
    char c = start[index];
    while(!is_delim(c)) {
        index++;
        c = start[index];
    }

    return make_slice(start, index);
}

void lex(Lexer* lexer, const char* path) {
    Str source = read_file(path);
    lexer->tokens = ALLOC(sizeof(Token) * source.length);

    size_t index = 0;
    while(index < source.length) {
        char current_char = char_at_string(&source, index);
        bool is_single = true;

        switch (current_char) {
            case ' ':
            case '\n':
            case '\t':
                index++;
                continue;
                break;
            case '+':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_PLUS);
                break;
            case '(':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_PAREN_OPEN);
                break;
            case ')':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_PAREN_CLOSE);
                break;
            case '{':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_BRACE_OPEN);
                break;
            case '}':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_BRACE_CLOSE);
                break;
            case '*':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_STAR);
                break;
            case '=':
                if(index + 1 < source.length && char_at_string(&source, index + 1) == '=') {
                    lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 2), TOKEN_EQUAL);
                    index += 2;
                    lexer->count++;
                    continue;
                }

                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_ASSIGN);
                break;
            case ';':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_SEMI_COLON);
                break;
            case ',':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_COMMA);
                break;
            case ':':
                if(index + 1 < source.length && char_at_string(&source, index + 1) == '=') {
                    lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 2), TOKEN_WALRUS);
                    index += 2;
                    lexer->count++;
                    continue;
                }

                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_COLON);
                break;
            case '^':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_HAT);
                break;
            case '@':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_AT);
                break;
            case '.':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_DOT);
                break;
            case '[':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_BRACKET_OPEN);
                break;
            case ']':
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_BRACKET_CLOSE);
                break;
            case '!':
                if(char_at_string(&source, index + 1) == '=') {
                    lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 2), TOKEN_NOT_EQUAL);
                    index += 2;
                    lexer->count++;
                    continue;
                }

                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[index], 1), TOKEN_NOT);
                break;
            case '"': {
                size_t start = index;
                index++;
                while(char_at_string(&source, index) != '"') {
                    if(index >= source.length) {
                        assert("Unexpected EOF string does not end... like my pain");
                    }
                    index++;
                }

                size_t length = (index - 1) - start;
                lexer->tokens[lexer->count] = make_token(make_slice(&source.buffer[start + 1], length), TOKEN_STRING_LITERAL);
                index++; // passover quote at the end
            }
                break;
            case '#': {
                while (current_char != '\n' && index < source.length) {
                    current_char = char_at_string(&source, index);
                    index++;
                }

                continue;
            }
                break;
            default:
                is_single = false;
        }

        if(is_single) {
            lexer->count++;
            index++;
            continue;
        }

        Slice word = get_word(&source.buffer[index]);
        index += word.length;

        bool is_keyword = true;

        if(compare_slice(&word, "return")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_RETURN);
        } else if(compare_slice(&word, "let")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_LET);
        } else if(compare_slice(&word, "insert")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_INSERT);
        } else if(compare_slice(&word, "fn")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_FN);
        } else if(compare_slice(&word, "loop")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_LOOP);
        }  else if(compare_slice(&word, "struct")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_STRUCT);
        } else if(compare_slice(&word, "new")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_NEW);
        } else if(compare_slice(&word, "break")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_BREAK);
        } else if(compare_slice(&word, "import")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_IMPORT);
        } else if(compare_slice(&word, "for")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_FOR);
        } else if(compare_slice(&word, "in")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_IN);
        } else if(compare_slice(&word, "false")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_FALSE);
        } else if(compare_slice(&word, "true")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_TRUE);
        } else if(compare_slice(&word, "if")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_IF);
        } else if(compare_slice(&word, "or")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_OR);
        } else if(compare_slice(&word, "and")) {
            lexer->tokens[lexer->count] = make_token(word, TOKEN_AND);
        } else {
            is_keyword = false;
        }

        if(!is_keyword) {
            int n = 0;
            if (slice_to_int(&word, &n)) {
                lexer->tokens[lexer->count] = make_token(word, TOKEN_INT_LITERAL);
            } else {
                lexer->tokens[lexer->count] = make_token(word, TOKEN_IDENTIFIER);
            }
        }

        lexer->count++;
    }
}

/*
 *  --- parser.h
 */
AstNode* make_node(NodeType type) {
    AstNode* node = ALLOC(sizeof(AstNode));
    node->node_type = type;
    return node;
}

Parser make_parser(Lexer* lexer) {
    Parser parser = {
            .tokens = lexer->tokens,
            .count = lexer->count,
            .current_token = 0,
            .root = ALLOC(sizeof(AstNode))
    };

    *(parser.root) = (AstNode) {
        .node_type = NODE_FILE,
        .data.file_node = {
            .sub_nodes = {}
        }
    };

    int sub_node_count = 0;

    while(parser.current_token < parser.count) {
        assert(sub_node_count < SUB_NODE_SIZE); // if this trigger good luck : ]
        AstNode* node = make_ast(&parser);
        parser.root->data.file_node.sub_nodes[sub_node_count] = node; // oh boy
        sub_node_count++;
    }

    return parser;
}

AstNode* make_ast(Parser* parser) {
    switch (peek(parser)->type) {
        case TOKEN_FN: return make_fn(parser); break;
        case TOKEN_LET: return make_assign(parser); break;
        default:
            assert(0);
    }
}

AstNode* make_assign(Parser* parser) {
    consume_token(parser, TOKEN_LET);
    Token* identifier = consume_token(parser, TOKEN_IDENTIFIER);
    consume_token(parser, TOKEN_ASSIGN);
    AstNode* assigned_to = make_expr_stmt(parser);

    AstNode* node = make_node(NODE_ASSIGNMENT);
    node->data.assignment_node = (AssignmentNode){
        .identifier = identifier->slice,
        .assigned_to = assigned_to
    };
    return node;
}

AstNode* make_fn(Parser* parser) {
    consume_token(parser, TOKEN_FN);
    Token* identifier_token = consume_token(parser, TOKEN_IDENTIFIER);
    consume_token(parser, TOKEN_PAREN_OPEN);
    consume_token(parser, TOKEN_PAREN_CLOSE);
    AstNode* body = make_scope(parser);
    AstNode* node  = make_node(NODE_FN_DECL);
    node->data.fn_node.identifier = identifier_token->slice;
    node->data.fn_node.body = body;
    return node;
}

AstNode* make_scope(Parser* parser) {
    consume_token(parser, TOKEN_BRACE_OPEN);
    AstNode* node = make_node(NODE_SCOPE);
    node->data.scope_node = (ScopeNode) {
        .sub_nodes = {}
    };

    // TODO: nested braces do not work with this
    int node_count = 0;
    while(peek(parser)->type != TOKEN_BRACE_CLOSE) {
        AstNode* sub_node = make_ast(parser);
        node->data.scope_node.sub_nodes[node_count] = sub_node;
        node_count++;
    }

    consume_token(parser, TOKEN_BRACE_CLOSE);
    return node;
}

AstNode* make_expr_stmt(Parser* parser) {
    AstNode* node = make_expr(parser);
    consume_token(parser, TOKEN_SEMI_COLON);
    return node;
}

AstNode* make_expr(Parser* parser) {
    return make_or(parser);
}

AstNode* make_or(Parser* parser) {
    AstNode* left = make_and(parser);

    if(peek(parser)->type == TOKEN_OR) {
        consume_token(parser, TOKEN_OR);
        AstNode* right = make_expr(parser);
        AstNode* binary = make_node(NODE_BINARY_EXPR);
        binary->data.binary_expr_node = (BinaryExprNode) {
            .left = left,
            .OpType = OP_OR,
            .right = right
        };

        return binary;
    }

    return left;
}

AstNode* make_and(Parser* parser) {
    AstNode* left = make_compare(parser);

    if(peek(parser)->type == TOKEN_AND) {
        consume_token(parser, TOKEN_AND);
        AstNode* right = make_expr(parser);
        AstNode* binary = make_node(NODE_BINARY_EXPR);
        binary->data.binary_expr_node = (BinaryExprNode) {
                .left = left,
                .OpType = OP_AND,
                .right = right
        };

        return binary;
    }

    return left;
}

AstNode* make_compare(Parser* parser) {
    AstNode* left = make_term(parser);

    if(peek(parser)->type == TOKEN_EQUAL) {
        consume_token(parser, TOKEN_EQUAL);
        AstNode* right = make_expr(parser);
        AstNode* binary = make_node(NODE_BINARY_EXPR);
        binary->data.binary_expr_node = (BinaryExprNode) {
                .left = left,
                .OpType = OP_EQUAL,
                .right = right
        };

        return binary;
    }

    return left;
}

AstNode* make_term(Parser* parser) {
    AstNode* left = make_factor(parser);

    if(peek(parser)->type == TOKEN_PLUS) {
        consume_token(parser, TOKEN_PLUS);
        AstNode* right = make_expr(parser);
        AstNode* binary = make_node(NODE_BINARY_EXPR);
        binary->data.binary_expr_node = (BinaryExprNode) {
                .left = left,
                .OpType = OP_ADD,
                .right = right
        };

        return binary;
    }

    return left;
}

AstNode* make_factor(Parser* parser) {
    AstNode* left = make_postfix(parser);

    if(peek(parser)->type == TOKEN_STAR) {
        consume_token(parser, TOKEN_STAR);
        AstNode* right = make_expr(parser);
        AstNode* binary = make_node(NODE_BINARY_EXPR);
        binary->data.binary_expr_node = (BinaryExprNode) {
                .left = left,
                .OpType = OP_MULT,
                .right = right
        };

        return binary;
    }

    return left;
}

AstNode* make_postfix(Parser* parser) {
    return make_call(parser);
}

AstNode* make_call(Parser* parser) {
    return make_array(parser);
}

AstNode* make_array(Parser* parser) {
    return make_primary(parser);
}

AstNode* make_primary(Parser* parser) {
    TokenType type = peek(parser)->type;
    if(type == TOKEN_INT_LITERAL) {
        Token* token = consume_token(parser, TOKEN_INT_LITERAL);
        AstNode* literal = make_node(NODE_INT_EXPR);
        literal->data.int_expr_node = (IntExprNode) {
                .literal = token->slice
        };
        return literal;
    }

    if(type == TOKEN_IDENTIFIER) {
        Token* token = consume_token(parser, TOKEN_IDENTIFIER);
        AstNode* literal = make_node(NODE_IDEN_EXPR);
        literal->data.iden_expr_node = (IdenExprNode) {
                .identifier = token->slice
        };
        return literal;
    }

    ASSERT(0, "This primary is not supported yet...");
}

AstNode* make_new(Parser* parser) {
    ASSERT(0, "Not implemented");
}

AstNode* make_group(Parser* parser) {
    ASSERT(0, "Not implemented");
}

 Token* peek(Parser* parser) {
    if(parser->current_token >= parser->count) {
        return NULL;
    }
    return &parser->tokens[parser->current_token];
}

Token* consume_token(Parser* parser, TokenType type) {
    Token* token = peek(parser);

    if(token == NULL) {
        ASSERT(0, "Unexpected end of tokens");
    } else if(token->type != type) {
        ASSERT(0, "Unexpected token");
    }

    parser->current_token++;
    return token;
}

void print_node(AstNode* node, int indent) {

    for(int i = 0; i < indent; i++) {
        printf(" ");
    }

    if(node->node_type == NODE_ASSIGNMENT) {
        print_slice(&node->data.assignment_node.identifier);
        printf(" = ");
        print_node(node->data.assignment_node.assigned_to, 0);
        printf("\n");
        return;
    }

    if(node->node_type == NODE_IDEN_EXPR) {
        print_slice(&node->data.iden_expr_node.identifier);
        return;
    }

    if(node->node_type == NODE_INT_EXPR) {
        print_slice(&node->data.int_expr_node.literal);
        return;
    }

    if(node->node_type == NODE_FN_DECL) {
        print_slice(&node->data.fn_node.identifier);
        printf("()\n");
        for(int i = 0; i < SUB_NODE_SIZE; i++) {
            AstNode* sub_node = node->data.fn_node.body->data.scope_node.sub_nodes[i];
            if(sub_node == NULL) continue;

            print_node(sub_node, indent + 3);
        }
        return;
    }

    if(node->node_type == NODE_BINARY_EXPR) {
        print_node(node->data.binary_expr_node.left, indent);
        switch (node->data.binary_expr_node.OpType) {
            case OP_ADD: printf(" + "); break;
            case OP_AND: printf(" and "); break;
            case OP_OR: printf(" or "); break;
            case OP_MULT: printf(" * "); break;
            case OP_EQUAL: printf(" == "); break;
            default: ASSERT(0, "This op is not supported to print yet..."); break;
        }
        print_node(node->data.binary_expr_node.right, indent);
        return;
    }

    ASSERT(0, "This node is not printable yet...")
}

void print_ast(Parser* parser) {
    assert(parser->root->node_type == NODE_FILE);
    for(int i = 0; i < SUB_NODE_SIZE; i++) {
        AstNode* sub_node = parser->root->data.file_node.sub_nodes[i];
        if(sub_node) {
            print_node(sub_node, 0);
        }
    }
}
