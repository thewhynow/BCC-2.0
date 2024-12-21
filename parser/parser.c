#include "parser.h"

#include <stdio.h>
#include <stdbool.h>

#if defined(_MSC_VER)
    #include <intrin.h>
    #define MARK_UNREACHABLE_CODE __assume(0)
#elif defined(__GNUC__)
    #define MARK_UNREACHABLE_CODE __builtin_unreachable()
#else
    #define MARK_UNREACHABLE_CODE abort(0)
#endif

static token_t* tokens;
static token_t curr_token;
static const char* fpath;
static node_t node;
static symbol_t symbol;

/* VARIABLE 'MAP' GLOBALS */
static symbol_t* global_scope;
static symbol_t* current_scope;
static size_t stack_frame_size;

static void parse_error(const char* msg){
    fprintf(stderr, "%s.%lu: %s\n", fpath, curr_token.line, msg);
    exit(-1);
}

static void advance_token(){
    static size_t token_count = 0;
    if (token_count < get_size_array(tokens))
        curr_token = tokens[token_count++];
    else {
        size_t line = curr_token.line;
        curr_token = (token_t){.type = TOK_NULL, .line = line};
    }
}

static node_t* make_node(node_t node){
    node_t* nod = malloc(sizeof(node_t));
    *nod = node;
    return nod;
}

node_t* parse(token_t* _tokens, const char* _fpath){
    tokens = _tokens;
    fpath = _fpath;
    advance_token();

    global_scope = alloc_array(sizeof(symbol_t), 1);
    current_scope = alloc_array(sizeof(symbol_t), 1);

    stack_frame_size = 0;

    node_t* code = alloc_array(sizeof(node_t), 1);

    node_t parse_top_level();

    while (curr_token.type != TOK_NULL){
        node = parse_top_level();
        pback_array(&code, &node);
    }

    return code;
}

node_t parse_top_level(){
    node_t result;
    /*static*/ node_t parse_expr();
/* 
    data_t = parse_type() 
    if (data_t) ...
*/
    if (curr_token.type == KEYW_int){
        advance_token();
        if (curr_token.type == TOK_IDENTIFIER){
            char* name = curr_token.identifier;
            advance_token();
            if (curr_token.type == TOK_OPEN_PAREN){
                symbol = (symbol_t){
                    .name = name,
                    .type = SYM_FUNC
                };
                pback_array(&global_scope, &symbol);
                
                /* dont need to parse function args */
                do
                    advance_token();
                while (curr_token.type != TOK_CLOSE_PAREN);

                advance_token();

                if (curr_token.type == TOK_OPEN_BRACE){
                    result.type = NOD_FUNC;
                    result.func_node = (nod_function_t){
                        .args = NULL,
                        .name = name,
                        /* can assume its block node */
                        .body = parse_expr().block_node
                    };
                    return result;
                }
                else
                    parse_error("Expected '{'");
            }
            else
                parse_error("This compiler does not support global variables ... yet");
        }
        else
            parse_error("Expected identifier");
    }
    else
        parse_error("Expected type name");

    MARK_UNREACHABLE_CODE;
}

node_t parse_expr(){
    node_t parse_expr_level_15();

    return parse_expr_level_15();
}

node_t parse_term(){
    node_t parse_fac();
    node_t result = parse_fac();
    while(true){
        switch(curr_token.type){
            case TOK_MUL: {
                advance_token();
                result = (node_t){
                    .type = NOD_BINARY_MUL,
                    
                    .binary_node.value_a = make_node(result),
                    .binary_node.value_b = make_node(parse_fac())
                };
                break;
            }

            case TOK_DIV: {
                advance_token();
                result = (node_t){
                    .type = NOD_BINARY_DIV,
                    
                    .binary_node.value_a = make_node(result),
                    .binary_node.value_b = make_node(parse_fac())
                };
                break;
            }

            case TOK_MOD: {
                advance_token();
                result = (node_t){
                    .type = NOD_BINARY_MOD,                    
                    .binary_node.value_a = make_node(result),
                    .binary_node.value_b = make_node(parse_fac())
                };
                break;
            }
            default:
                return result;
        }
    }
}

node_t parse_fac(){
    
    node_t parse_expr();

    switch(curr_token.type){
        case TOK_INTEGER: {
            unsigned int val = curr_token.uint_literal;
            advance_token();
            return (node_t){
                .type = NOD_INTEGER,
                .int_node.val = val
            };
        }

        case TOK_OPEN_BRACE: {
            advance_token();

            node_t* code = alloc_array(sizeof(node_t), 1);

            while (curr_token.type != TOK_CLOSE_BRACE){
                node = parse_expr();
                pback_array(&code, &node);
                if (curr_token.type == TOK_SEMICOLON)
                    advance_token();
                else
                    parse_error("Expected expression-terminator: '}' or ';'");
            }

            advance_token();

            return (node_t){
                .type = NOD_BLOCK,
                .block_node = (nod_block_t){
                    .code = code,
                    .locals = current_scope
                }
            };
        }

        case TOK_ADD: {
            advance_token();
            /* we can do this because the unary '+' operator quite literally does nothing */
            return parse_fac();
        }

        case TOK_SUB: {
            advance_token();
            return (node_t){
                .type = NOD_UNARY_SUB,
                .unary_node = (nod_unary_t){
                    .value = make_node(parse_fac())
                }
            };
        }

        case TOK_BITWISE_NOT: {
            advance_token();
            return (node_t){
                .type = NOD_BITWISE_NOT,
                .unary_node = (nod_unary_t){
                    .value = make_node(parse_fac())
                }
            };
        }

        case TOK_LOGICAL_NOT: {
            advance_token();
            return (node_t){
                .type = NOD_LOGICAL_NOT,
                .unary_node = (nod_unary_t){
                    .value = make_node(parse_fac())
                }
            };
        }

        case TOK_OPEN_PAREN: {
            advance_token();
            node_t expression = parse_expr();

            if (curr_token.type == TOK_CLOSE_PAREN){
                advance_token();
                return expression;
            }
            else
                parse_error("Unterminated parenthesis expression");
        }

        case KEYW_return: {
            advance_token();
            return (node_t){
                .type = NOD_RETURN,
                .return_node = (nod_return_t){
                    .value = make_node(parse_expr())
                }
            };
        }

        default: {
            parse_error("Unexpected token");
        }
    }

    MARK_UNREACHABLE_CODE;
}

/* addition and subtraction */
node_t parse_expr_level_4(){
    node_t result = parse_term();
    while (true){
        switch(curr_token.type){
            case TOK_ADD: {
                advance_token();
                result = (node_t){
                    .type = NOD_BINARY_ADD,
                    
                    .binary_node.value_a = make_node(result),
                    .binary_node.value_b = make_node(parse_term())
                };
                break;
            }

            case TOK_SUB: {
                advance_token();
                result = (node_t){
                    .type = NOD_BINARY_SUB,
                    
                    .binary_node.value_a = make_node(result),
                    .binary_node.value_b = make_node(parse_term())
                };
                break;
            }

            default:
                return result;
        }
    }
}

/* bitwise left and right shift */
node_t parse_expr_level_5(){
    node_t result = parse_expr_level_4();
    while (true){
        switch(curr_token.type){
            case TOK_SHIFT_LEFT:
                advance_token();
                result = (node_t){
                    .type = NOD_SHIFT_LEFT,
                    .binary_node.value_a = make_node(result),
                    .binary_node.value_b = make_node(parse_expr_level_4())
                };
                break;

            case TOK_SHIFT_RIGHT:
                advance_token();
                result = (node_t){
                    .type = NOD_SHIFT_RIGHT,
                    .binary_node.value_a = make_node(result),
                    .binary_node.value_b = make_node(parse_expr_level_4())
                };
                break;

            default:
                return result;
        }
    }

}

/* <, <=, >, >= */
node_t parse_expr_level_6(){
    node_t result = parse_expr_level_5();
    while (true){
        switch(curr_token.type){
            case TOK_LESS_THAN:
                advance_token();
                result = (node_t){
                    .type = NOD_LESS_THAN,
                    .binary_node.value_a = make_node(result),
                    .binary_node.value_b = make_node(parse_expr_level_5())
                };
                break;
            
            case TOK_GREATER_THAN:
                advance_token();
                result = (node_t){
                    .type = NOD_GREATER_THAN,
                    .binary_node.value_a = make_node(result),
                    .binary_node.value_b = make_node(parse_expr_level_5())
                };
                break;

            case TOK_LESS_THAN_OR_EQUAL:
                advance_token();
                result = (node_t){
                    .type = NOD_LESS_THAN_OR_EQUAL,
                    .binary_node.value_a = make_node(result),
                    .binary_node.value_b = make_node(parse_expr_level_5())
                };
                break;

            case TOK_GREATER_THAN_OR_EQUAL:
                advance_token();
                result = (node_t){
                    .type = NOD_GREATER_THAN_OR_EQUAL,
                    .binary_node.value_a = make_node(result),
                    .binary_node.value_b = make_node(parse_expr_level_5())
                };
                break;

            default:
                return result;
        }
    }
}

/* == and != */
node_t parse_expr_level_7(){
    node_t result = parse_expr_level_6();
    while (true){
        switch(curr_token.type){
            case TOK_EQUAL_TO:
                advance_token();
                result = (node_t){
                    .type = NOD_EQUAL_TO,
                    .binary_node.value_a = make_node(result),
                    .binary_node.value_b = make_node(parse_expr_level_6())
                };
                break;
            
            case TOK_NOT_EQUAL_TO:
                advance_token();
                result = (node_t){
                    .type = NOD_NOT_EQUAL_TO,
                    .binary_node.value_a = make_node(result),
                    .binary_node.value_b = make_node(parse_expr_level_6())
                };
                break;
            

            default:
                return result;
        }
    }
}

/* bitwise AND */
node_t parse_expr_level_8(){
    node_t result = parse_expr_level_7();
    while (true){
        switch(curr_token.type){
            case TOK_BITWISE_AND:
                advance_token();
                result = (node_t){
                    .type = NOD_BITWISE_AND,
                    .binary_node.value_a = make_node(result),
                    .binary_node.value_b = make_node(parse_expr_level_7())
                };
                break;
            default:
                return result;
        }
    }
}

/* bitwise XOR */
node_t parse_expr_level_9(){
    node_t result = parse_expr_level_8();
    while (true){
        switch(curr_token.type){
            case TOK_BITWISE_XOR:
                advance_token();
                result = (node_t){
                    .type = NOD_BITWISE_XOR,
                    .binary_node.value_a = make_node(result),
                    .binary_node.value_b = make_node(parse_expr_level_8())
                };
                break;
            default:
                return result;
        }
    }
}

/* bitwise OR */
node_t parse_expr_level_10(){
    node_t result = parse_expr_level_9();
    while (true){
        switch(curr_token.type){
            case TOK_BITWISE_OR:
                advance_token();
                result = (node_t){
                    .type = NOD_BITWISE_OR,
                    .binary_node.value_a = make_node(result),
                    .binary_node.value_b = make_node(parse_expr_level_9())
                };
                break;
            default:
                return result;
        }
    }
}

/* logical AND */
node_t parse_expr_level_11(){
    node_t result = parse_expr_level_10();
    while (true){
        switch(curr_token.type){
            case TOK_LOGICAL_AND:
                advance_token();
                result = (node_t){
                    .type = NOD_LOGICAL_AND,
                    .binary_node.value_a = make_node(result),
                    .binary_node.value_b = make_node(parse_expr_level_10())
                };
                break;
            default:
                return result;
        }
    }
}

/* logical OR */
node_t parse_expr_level_12(){
    node_t result = parse_expr_level_11();
    while (true){
        switch(curr_token.type){
            case TOK_LOGICAL_OR:
                advance_token();
                result = (node_t){
                    .type = NOD_LOGICAL_OR,
                    .binary_node.value_a = make_node(result),
                    .binary_node.value_b = make_node(parse_expr_level_11())
                };
                break;
            default:
                return result;
        }
    }
}

/* ternary operator */
node_t parse_expr_level_13(){
    node_t result = parse_expr_level_12();
    while (true){
        switch(curr_token.type){
            default:
                return result;
        }
    }
}

/* assignment */
node_t parse_expr_level_14(){
    node_t result = parse_expr_level_13();
    while (true){
        switch(curr_token.type){
            default:
                return result;
        }
    }
}

/* comma operator */
node_t parse_expr_level_15(){
    node_t result = parse_expr_level_14();
    while (true){
        switch(curr_token.type){
            default:
                return result;
        }
    }
}


void parse_node_dstr(void* _node){
    node_t* node = _node;
    switch(node->type){
        case NOD_NULL:
            return;
        case NOD_INTEGER:
            return;
        
        case NOD_RETURN:
            parse_node_dstr(node->return_node.value);
            return;
        
        case NOD_BLOCK:
            free_array(node->block_node.code, parse_node_dstr);
            free_array(node->block_node.locals, NULL);

            return;

        case NOD_FUNC:
            free_array(node->func_node.body.code, parse_node_dstr);
            free_array(node->func_node.body.locals, NULL);

            return;

        case NOD_LOGICAL_NOT: 
        case NOD_UNARY_SUB: 
        case NOD_BITWISE_NOT:
            parse_node_dstr(node->unary_node.value);
            return;

        case NOD_BINARY_ADD:
        case NOD_BINARY_SUB:
        case NOD_BINARY_MUL:
        case NOD_BINARY_DIV:
        case NOD_BINARY_MOD:
        case NOD_BITWISE_AND:
        case NOD_BITWISE_OR:
        case NOD_BITWISE_XOR:
        case NOD_SHIFT_LEFT:
        case NOD_SHIFT_RIGHT:
        case NOD_LOGICAL_AND:
        case NOD_LOGICAL_OR:
        case NOD_EQUAL_TO:
        case NOD_NOT_EQUAL_TO:
        case NOD_LESS_THAN:
        case NOD_GREATER_THAN:
        case NOD_LESS_THAN_OR_EQUAL:
        case NOD_GREATER_THAN_OR_EQUAL:
            parse_node_dstr(node->binary_node.value_a);
            parse_node_dstr(node->binary_node.value_b);
            return;
    }
}