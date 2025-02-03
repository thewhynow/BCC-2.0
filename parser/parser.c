#include "parser.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#if defined(__GNUC__)
    #define MARK_UNREACHABLE_CODE __builtin_unreachable()
#else
    #define MARK_UNREACHABLE_CODE abort(0)
#endif

#define WARN_EMPTY_EXPRESSION 0

static token_t* tokens =    NULL;
static token_t curr_token = {0};
static const char* fpath =  NULL;
static node_t node =        {0};

/* VARIABLE 'MAP' GLOBALS */
scope_t* global_scope =   NULL;
static scope_t* current_scope =  NULL;
static size_t stack_frame_size = 0;
static size_t __scope_id =       1;

/* 
* a c-vec of all the local scopes defined in a function
* not static so can be accessed during ir-gen
*/
symbol_t** function_scope = NULL;

static scope_t* _temp_scope =  NULL;

static void parse_error(const char* msg){
    fprintf(stderr, "ERROR: %s.%lu: %s\n", fpath, curr_token.line, msg);
    exit(-1);
}

void data_t_dstr(void* _data){
    if (((data_type_t*)_data)->base_type == TYPE_POINTER || ((data_type_t*)_data)->base_type == TYPE_ARRAY){
        data_t_dstr(((data_type_t*)_data)->ptr_derived_type);
        free(((data_type_t*)_data)->ptr_derived_type);
    }
}

data_type_t* alloc_data_t(data_type_t type){
    data_type_t* ptr = malloc(sizeof(data_type_t));
    *ptr = type;
    return ptr;
}

void symbol_dstr(void* _symbol){
    symbol_t* symbol = _symbol;

    if (symbol->symbol_type != SYM_VARIADIC){
        data_t_dstr(&symbol->data_type);
        if (symbol->symbol_type == SYM_FUNCTION)
            free_array(symbol->args, symbol_dstr); 
    }
    return;
}

symbol_t clone_symbol_t(symbol_t symbol){
    
    symbol_t result;
    data_type_t clone_data_t(data_type_t src);

    if (symbol.symbol_type == SYM_FUNCTION){
        symbol_t* copied_args = alloc_array(sizeof(symbol_t), 1);

        for (size_t i = 0; i < get_count_array(symbol.args); ++i){
            result = clone_symbol_t(symbol.args[i]);
            pback_array(&copied_args, &result);
        }


        return (symbol_t){
            .args = copied_args,
            .data_type = clone_data_t(symbol.data_type),
            .name = symbol.name,
            .symbol_type = SYM_FUNCTION
        };
    }
    else {
        result = symbol;
        result.data_type = clone_data_t(symbol.data_type);

        return result;
    }
}

static size_t token_count = 0;
static void advance_token(){
    if (token_count < get_count_array(tokens))
        curr_token = tokens[token_count++];
    else {
        size_t line = curr_token.line;
        curr_token = (token_t){.type = TOK_NULL, .line = line};
    }
}

node_t* make_node(node_t node){
    node_t* nod = malloc(sizeof(node_t));
    *nod = node;
    return nod;
}

bool symbol_comp(void* _symbol_a, void* _symbol_b){
    symbol_t* symbol_a = _symbol_a;
    symbol_t* symbol_b = _symbol_b;

    return !strcmp(symbol_a->name, symbol_b->name) && (symbol_a->symbol_type == symbol_b->symbol_type);
}

#define _find_symbol_array(array, target) (array_search((array), (&(target)), symbol_comp))

void enter_scope() {
    _temp_scope = current_scope;
    current_scope = malloc(sizeof(scope_t));
    current_scope->above_scope = _temp_scope;
    current_scope->top_scope = alloc_array(sizeof(symbol_t), 1);
    current_scope->scope_id = __scope_id++;
}

/* dont free the symbol array as it is used later */
void exit_scope(){
    size_t symbol_arr_count = get_count_array(function_scope);
    if (symbol_arr_count  == current_scope->scope_id)
        pback_array(&function_scope, &current_scope->top_scope);
    else
    if (symbol_arr_count < current_scope->scope_id)
        while (get_count_array(function_scope) <= current_scope->scope_id)
            /* first lvalue i could think of */
            pback_array(&function_scope, &current_scope->top_scope);
    else
        function_scope[current_scope->scope_id] = current_scope->top_scope;

    _temp_scope = current_scope->above_scope;
    free(current_scope);
    current_scope = _temp_scope;
}

symbol_t* find_symbol(scope_t* scope, symbol_t target, size_t* scope_id_out, size_t* var_num_out){
    symbol_t* symbol = _find_symbol_array(scope->top_scope, target);
    if (symbol){
        if (scope_id_out)
            *scope_id_out = scope->scope_id;
        if (var_num_out)
            *var_num_out = symbol - scope->top_scope;
        return symbol;
    }
    else
        if (scope->above_scope)
            return find_symbol(scope->above_scope, target, scope_id_out, var_num_out);
        else
            return NULL;
}

node_t* parse(token_t* _tokens, const char* _fpath){
    tokens = _tokens;
    fpath = _fpath;
    advance_token();

    global_scope = malloc(sizeof(scope_t));
    global_scope->above_scope = NULL;
    global_scope->top_scope = alloc_array(sizeof(symbol_t), 1);
    global_scope->scope_id = 0;

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
    
    data_type_t parse_type();
    symbol_t* parse_function_args();
    node_t parse_expr();

    data_type_t d_type = parse_type();

    if (d_type.base_type){

        symbol_t parse_declaration(data_type_t d_type);

        symbol_t symbol = parse_declaration(d_type);


        if (symbol.symbol_type == SYM_FUNCTION){

            for (size_t i = 0; i < get_count_array(symbol.args); ++i)
                if (symbol.args[i].data_type.base_type == TYPE_ARRAY)
                    symbol.args[i].data_type.base_type = TYPE_POINTER;
            
            pback_array(&global_scope->top_scope, &symbol);

            if (curr_token.type == TOK_OPEN_BRACE){
                function_scope = alloc_array(sizeof(symbol_t*), 1);
                pback_array(&function_scope, &global_scope->top_scope);
                __scope_id = 1;
                /* parse block node code, but slightly modified */
                    advance_token();

                    node_t* code = alloc_array(sizeof(node_t), 1);

                    enter_scope();
                    
                    /* easiest way to do this */
                    current_scope->above_scope = global_scope;

                    for (size_t i = 0; i < get_count_array(symbol.args); ++i)
                        if (symbol.args[i].symbol_type != SYM_VARIADIC){
                            symbol_t arg = clone_symbol_t(symbol.args[i]);
                            pback_array(&current_scope->top_scope, &arg);
                        }

                    while (curr_token.type != TOK_CLOSE_BRACE){
                        node = parse_expr();
                        pback_array(&code, &node);
                        if (curr_token.type == TOK_SEMICOLON || curr_token.type == TOK_CLOSE_BRACE)
                            advance_token();
                        else
                            parse_error("expected expression-terminator: '}' or ';'");
                    }

                    size_t this_scope_id = current_scope->scope_id;

                    exit_scope();
                /* --- */

                result.type = NOD_FUNC;
                result.func_node = (nod_function_t){
                    .args = symbol.args,
                    .name = symbol.name,
                    .function_scope = function_scope,
                    .type = d_type,
                    /* can assume its block node */
                    .body = (nod_block_t){
                        .code = code,
                        .scope_id = this_scope_id
                    },
                };

                advance_token(); /* advance past last '}' */

                return result;
            } else
            if (curr_token.type == TOK_SEMICOLON){
                advance_token();
                return (node_t){
                    .type = NOD_NULL
                };
            }
            else
                parse_error("expected ';' or '}'");
            
        }
        else {
            if (curr_token.type == TOK_SEMICOLON){
                symbol.init = false;
                pback_array(&global_scope->top_scope, &symbol);
                advance_token();
                return (node_t){0};
            } else
            if (curr_token.type == TOK_ASSIGN){
                advance_token();
                symbol.init = true;
                pback_array(&global_scope->top_scope, &symbol);
                node = (node_t){
                    .type = NOD_STATIC_INIT,
                    .static_init_node  = (nod_static_init_t){
                        .sym_info = symbol,
                        .value = make_node(parse_expr()),
                        .global = true,
                    }
                };
                advance_token();
                return node;
            }
            else
                parse_error("expected ';'");
        }
    }
    else
        parse_error("expected type name");

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
    data_type_t parse_type();

    /* switch case handling */
    static bool in_switch = false;
    static bool default_case_defined = false;
    static node_t* switch_cases = NULL;
    static size_t case_num = 0;
    data_type_t d_type = parse_type();

    /* declaration */
    if (d_type.base_type){
        
        symbol_t parse_declaration(data_type_t d_type);

        symbol_t symbol = parse_declaration(d_type);

        if (curr_token.type == TOK_SEMICOLON){
            /* variable declarations dont do anything on their own */
            symbol.init = false;
            pback_array(&current_scope->top_scope, &symbol);
            return (node_t){
                .type = NOD_NULL
            };
        } else
        if (curr_token.type == TOK_ASSIGN){
            advance_token();
            symbol.init = true;
            pback_array(&current_scope->top_scope, &symbol);
            if (symbol.data_type.storage_class == STORAGE_STATIC){
                return (node_t){
                    .type = NOD_STATIC_INIT,
                    .static_init_node = (nod_static_init_t){
                        .sym_info = symbol,
                        .value = make_node(parse_expr()),
                        .global = false,
                    }
                };
            }
            else {
                if (symbol.data_type.base_type == TYPE_ARRAY)
                    return (node_t){
                        .type = NOD_INIT_ARRAY,
                        .array_init_node.array_var = (var_info_t){
                            .var_num = find_symbol(current_scope, symbol, NULL, NULL) - current_scope->top_scope,
                            .scope_id = current_scope->scope_id,
                        },
                        .array_init_node.elems = make_node(parse_expr())
                    };

                else
                    return (node_t){
                        .type = NOD_ASSIGN,
                        .assign_node = (nod_assign_t){
                            .destination = make_node((node_t){
                                .type = NOD_VARIABLE_ACCESS,
                                .var_access_node = (nod_var_access_t){
                                    .var_info = (var_info_t){
                                        .var_num = find_symbol(current_scope, symbol, NULL, NULL) - current_scope->top_scope,
                                        .scope_id = current_scope->scope_id,
                                    }
                                }
                            }),
                            .source = make_node(parse_expr())
                        }
                    };
            }

        }
        else
            parse_error("expected '=' or ';' after variable declaration");
    }
    else {
        node_t result;
        switch(curr_token.type){

            case TOK_INTEGER: {
                unsigned int val = curr_token.literals.uint;
                advance_token();
                result = (node_t){
                    .type = NOD_INTEGER,
                    .const_node.int_literal = val,
                };

                break;
            }

            case TOK_LONG: {
                unsigned long val = curr_token.literals.ulong;
                advance_token();
                result = (node_t){
                    .type = NOD_LONG,
                    .const_node.long_literal = val,
                };

                break;
            }

            case TOK_SHORT: {
                unsigned short val = curr_token.literals.ushort;
                advance_token();
                result = (node_t){
                    .type = NOD_SHORT,
                    .const_node.short_literal = val,
                };

                break;
            }

            case TOK_CHAR: {
                unsigned char val = curr_token.literals.uchar;
                advance_token();
                result = (node_t){
                    .type = NOD_CHAR,
                    .const_node.char_literal = val,
                };

                break;
            }

            case TOK_UINTEGER: {
                unsigned int val = curr_token.literals.uint;
                advance_token();
                result = (node_t){
                    .type = NOD_UINTEGER,
                    .const_node.int_literal = val
                };

                break;
            }

            case TOK_ULONG: {
                unsigned int val = curr_token.literals.ulong;
                advance_token();
                result = (node_t){
                    .type = NOD_ULONG,
                    .const_node.long_literal = val
                };

                break;
            }

            case TOK_USHORT: {
                unsigned int val = curr_token.literals.ushort;
                advance_token();
                result = (node_t){
                    .type = NOD_USHORT,
                    .const_node.short_literal = val
                };

                break;
            }

            case TOK_UCHAR: {
                unsigned int val = curr_token.literals.uchar;
                advance_token();
                result = (node_t){
                    .type = NOD_UCHAR,
                    .const_node.char_literal = val
                };

                break;
            }

            case TOK_OPEN_BRACE: {
                advance_token();

                node_t* code = alloc_array(sizeof(node_t), 1);
                
                enter_scope();

                while (curr_token.type != TOK_CLOSE_BRACE){
                    node = parse_expr();
                    pback_array(&code, &node);
                    if (curr_token.type == TOK_SEMICOLON || curr_token.type == TOK_CLOSE_BRACE)
                        advance_token();
                    else
                        parse_error("expected expression-terminator: '}' or ';'");
                }

                size_t this_scope_id = current_scope->scope_id;

                exit_scope();

                return (node_t){
                    .type = NOD_BLOCK,
                    .block_node = (nod_block_t){
                        .code = code,
                        .scope_id = this_scope_id
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
                result = (node_t){
                    .type = NOD_UNARY_SUB,
                    .unary_node = (nod_unary_t){
                        .value = make_node(parse_fac())
                    }
                };

                break;
            }

            case TOK_BITWISE_NOT: {
                advance_token();
                result = (node_t){
                    .type = NOD_BITWISE_NOT,
                    .unary_node = (nod_unary_t){
                        .value = make_node(parse_fac())
                    }
                };

                break;
            }

            case TOK_LOGICAL_NOT: {
                advance_token();
                result = (node_t){
                    .type = NOD_LOGICAL_NOT,
                    .unary_node = (nod_unary_t){
                        .value = make_node(parse_fac())
                    }
                };

                break;
            }

            case TOK_INCREMENT: {
                advance_token();
                result = (node_t){
                    .type = NOD_PRE_INCREMENT,
                    .unary_node = (nod_unary_t){
                        .value = make_node(parse_fac())
                    }
                };

                break;
            }

            case TOK_OPEN_PAREN: {
                advance_token();
                
                data_type_t parse_anonymous_declaration(data_type_t d_type);

                data_type_t cast_type = parse_type();

                if (cast_type.base_type){
                    cast_type = parse_anonymous_declaration(cast_type);
                    if (!cast_type.storage_class){
                        if (curr_token.type == TOK_CLOSE_PAREN){
                            advance_token();
                            /* compound literal */
                            if (curr_token.type == TOK_OPEN_BRACE){
                                if (cast_type.base_type == TYPE_ARRAY){
                                    node_t parse_array_literal(data_type_t type);

                                    node_t elems = parse_array_literal(cast_type);

                                    data_t_dstr(&elems.array_literal_node.type);

                                    advance_token();

                                    if (elems.type == NOD_ARRAY_LITERAL){
                                        return (node_t){
                                            .type = NOD_ARRAY_LITERAL,
                                            .array_literal_node.elems = elems.array_literal_node.elems,
                                            .array_literal_node.type = cast_type
                                        };;
                                    }   
                                    else
                                        parse_error("can only define array variable with array literal");
                                }
                                else
                                    parse_error("only scalar initializers of array type are supported");
                                
                            }
                            else {
                                result = (node_t){
                                    .type = NOD_TYPE_CAST,
                                    .type_cast_node = (nod_type_cast_t){
                                        .dst_type = cast_type,
                                        .src = make_node(parse_fac())
                                    }
                                };
                                break;
                            }
                        }
                        else
                            parse_error("expected ')' after typecast");
                    }
                    else
                        parse_error("cannot specify storage class in cast expression");
                }

                node_t expression = parse_expr();

                if (curr_token.type == TOK_CLOSE_PAREN){
                    advance_token();
                    return expression;
                }
                else
                    parse_error("unterminated parenthesis expression");
            }

            case TOK_IDENTIFIER: {
                size_t this_scope_id;
                size_t this_var_num;
                symbol_t* symbol;
                char* identifier = curr_token.identifier;
                advance_token();

                /* function call - should really be handled somewhere else but i dont wanna deal with allat */
                if (curr_token.type == TOK_OPEN_PAREN){
                    node_t* func_args = alloc_array(sizeof(node_t), 1);
                    advance_token();
                    
                    while (curr_token.type != TOK_CLOSE_PAREN){
                        node_t node = parse_expr();
                        pback_array(&func_args, &node);
                        if (curr_token.type == TOK_COMMA)
                            advance_token(); else
                        if (curr_token.type == TOK_CLOSE_PAREN)
                            continue;
                        else
                            parse_error("expected ',' or ')'");
                    }
                    advance_token();

                    result = (node_t){
                        .type = NOD_FUNC_CALL,
                        .func_call_node = (nod_function_call_t){
                            .args = func_args,
                            .identifier = identifier,
                            .storage_class = d_type.storage_class
                        }
                    };

                    break;
                }
                else {
                    symbol = find_symbol(current_scope, ((symbol_t){.name = identifier, .symbol_type = SYM_VARIABLE}), &this_scope_id, &this_var_num);
                    if (symbol){
                        result = (node_t){
                            .type = NOD_VARIABLE_ACCESS,
                            .var_access_node.var_info.var_num = this_var_num,
                            .var_access_node.var_info.scope_id = this_scope_id,
                        };
                        break;
                    }
                    else
                        parse_error("attempt to use non-existing variable");
                }

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

            case KEYW_if: {
                advance_token();
                if (curr_token.type == TOK_OPEN_PAREN){
                    node_t condition = parse_fac();
                    node_t if_block = parse_expr();
                    if (((token_count + 1) < get_count_array(tokens)) && tokens[token_count].type == KEYW_else){
                        advance_token();
                        advance_token();
                        node_t else_block = parse_expr();
                        return (node_t){
                            .type = NOD_IF_STATEMENT,
                            .conditional_node = (nod_conditional_t){
                                .condition = make_node(condition),
                                .true_block = make_node(if_block),
                                .false_block = make_node(else_block)
                            }
                        };
                    }
                    else 
                        return (node_t){
                            .type = NOD_IF_STATEMENT,
                            .conditional_node = (nod_conditional_t){
                                .condition = make_node(condition),
                                .true_block = make_node(if_block),
                                .false_block = NULL
                            }
                        };
                }
                else
                    parse_error("expected '('");
            }

            case KEYW_while: {
                advance_token();
                if (curr_token.type == TOK_OPEN_PAREN) {
                    node_t condition = parse_fac();
                    node_t code = parse_expr();
                    return (node_t){
                        .type = NOD_WHILE_LOOP,
                        .while_node = (nod_while_t){
                            .condition = make_node(condition),
                            .code = make_node(code),
                        }
                    };
                }
                else
                    parse_error("expected '('");
            }

            case KEYW_do: {
                advance_token();
                node_t code = parse_expr();
                /* advance past expression - terminator token */
                advance_token();
                if (curr_token.type == KEYW_while){
                    advance_token();
                    if (curr_token.type == TOK_OPEN_PAREN){
                        node_t condition = parse_fac();
                        return (node_t){
                            .type = NOD_DO_WHILE_LOOP,
                            .while_node.code = make_node(code),
                            .while_node.condition = make_node(condition)
                        };
                    }
                    else
                        parse_error("expected '('");
                }   
                else
                    parse_error("expected 'while'");
            }

            case KEYW_for: {
                /* since for loops create new scopes by themselves */
                enter_scope();

                advance_token();
                if (curr_token.type == TOK_OPEN_PAREN){
                    advance_token();
                    node_t init = parse_expr();
                    if (curr_token.type == TOK_SEMICOLON){
                        advance_token();
                        node_t condition = parse_expr();
                        if (curr_token.type == TOK_SEMICOLON){
                            advance_token();
                            node_t repeat = parse_expr();
                            if (curr_token.type == TOK_CLOSE_PAREN){
                                advance_token();
                                node_t code = parse_expr();

                                size_t this_scope_id = current_scope->scope_id;

                                exit_scope();

                                return (node_t){
                                    .type = NOD_FOR_LOOP,
                                    .for_node = (nod_for_loop_t){
                                        .init = make_node(init),
                                        .condition = make_node(condition),
                                        .repeat = make_node(repeat),
                                        .code = make_node(code),
                                        .scope_id = this_scope_id
                                    }
                                };
                            }
                            else
                                parse_error("expected ')'");
                        }
                        else
                            parse_error("expected ';'");
                    }
                    else
                        parse_error("expected ';'");
                }
                else
                    parse_error("expected '('");
            }

            case KEYW_break: {
                advance_token();
                return (node_t){
                    .type = NOD_BREAK,
                };
            }

            case KEYW_continue: {
                advance_token();
                return (node_t){
                    .type = NOD_CONTINUE,
                };
            }

            case TOK_SEMICOLON: {
                #if WARN_EMPTY_EXPRESSION
                    printf("WARNING: %s.%lu: empty expression or syntax error\n", fpath, curr_token.line);
                #endif
                return (node_t){
                    .type = NOD_NULL
                };
            }

            case TOK_LABEL: {
                /* so the "expression" ends with a valid token */
                curr_token.type = TOK_SEMICOLON;
                return (node_t){
                    .type = NOD_LABEL,
                    .label_node.label = curr_token.identifier
                };
            }

            case KEYW_goto: {
                advance_token();
                if (curr_token.type == TOK_IDENTIFIER){
                    char* identifier = curr_token.identifier;
                    advance_token();
                    return (node_t){
                        .type = NOD_GOTO,
                        .label_node.label = identifier
                    };
                }
                else
                    parse_error("expected identifier");
            }

            case KEYW_switch: {
                advance_token();
                if (curr_token.type == TOK_OPEN_PAREN){
                    node_t value = parse_fac();
                    if (curr_token.type == TOK_OPEN_BRACE){
                        bool old_in_switch = in_switch;
                        bool old_default_case_defined = default_case_defined;
                        in_switch = true;
                        node_t* old_switch_cases = switch_cases;
                        switch_cases = alloc_array(sizeof(node_t), 1);
                        size_t old_case_num = case_num;
                        case_num = 0;

                        nod_block_t code = parse_fac().block_node;
                        
                        node_t node = (node_t){
                            .type = NOD_SWITCH_STATEMENT,
                            .switch_node = (nod_switch_case_t){
                                .cases = switch_cases,
                                .code = code,
                                .value = make_node(value),
                                .default_case_defined = default_case_defined
                            }
                        };

                        in_switch = old_in_switch;
                        switch_cases = old_switch_cases;
                        case_num = old_case_num;
                        default_case_defined = old_default_case_defined;

                        return node;
                    }
                    else
                        parse_error("expected '{'");
                }   
                else
                    parse_error("expected '('");
            }

            case KEYW_case: {
                if (switch_cases){
                    advance_token();
                    node_t value = parse_expr();

                    pback_array(&switch_cases, &value);

                    if (curr_token.type == TOK_COLON){
                        /* so it ends in a valid expression-terminator */
                        curr_token.type = TOK_SEMICOLON;

                        return (node_t){
                            .type = NOD_SWITCH_CASE,
                            .case_node = (nod_case_t){
                                .case_id = case_num++
                            }
                        };
                    }
                    else
                        parse_error("expected ':'");
                }
                else
                    parse_error("cannot use 'case' keyword outside of switch statement");
            }

            case KEYW_default: {
                if (switch_cases){
                    default_case_defined = true;
                    advance_token();
                    if (curr_token.type == TOK_COLON){
                        curr_token.type = TOK_SEMICOLON;
                        return (node_t){
                            .type = NOD_DEFAULT_CASE
                        };
                    }
                    else
                        parse_error("expected ':'");
                }
                else
                    parse_error("cannot use 'default' keyword outside of switch statement");
            }

            case TOK_BITWISE_AND: {
                advance_token();
                result = (node_t){
                    .type = NOD_REFERENCE,
                    .unary_node.value = make_node(parse_fac())
                };

                break;
            }

            case TOK_MUL: {
                advance_token();
                result = (node_t){
                    .type = NOD_DEREFERENCE,
                    .unary_node.value = make_node(parse_fac())
                };

                break;
            }

            case TOK_STRING_LITERAL: {
                char* str = curr_token.identifier;
                advance_token();
                return (node_t){
                    .type = NOD_STRING_LITERAL,
                    .const_node.str_literal = str
                };
            }

            case KEYW_sizeof: {
                advance_token();
                if (curr_token.type == TOK_OPEN_PAREN){
                    advance_token();
                    data_type_t type = parse_type();
                    if (type.base_type){
                        if (curr_token.type == TOK_CLOSE_PAREN){
                            advance_token();
                            return (node_t){
                                .type = NOD_SIZEOF_TYPE,
                                .sizeof_type_node.d_type = type
                            };
                        }
                        else
                            parse_error("expected ')'");
                    }
                    else
                        parse_error("expected type name");
                }
                else {
                    node_t node = parse_fac();
                    return (node_t){
                        .type = NOD_SIZEOF_EXPR,
                        .unary_node.value = make_node(node)
                    };
                }
            }

            default: {
                parse_error("unexpected token");
            }
        }

        if (curr_token.type == TOK_INCREMENT){
            advance_token();
            result = (node_t){
                .type = NOD_POST_INCREMENT,
                .unary_node.value = make_node(result)
            };
        }
        else {
            while (curr_token.type == TOK_OPEN_BRACKET){
                advance_token();
                
                result = (node_t){
                    .type = NOD_SUBSCRIPT,
                    .binary_node.value_a = make_node(result),
                    .binary_node.value_b = make_node(parse_expr())
                };

                if (curr_token.type == TOK_CLOSE_BRACKET)
                    advance_token();
                else
                    parse_error("expected ']'");
            }
        }

        return result;
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
            case TOK_QUESTION_MARK: {
                advance_token();
                node_t true_val = parse_expr();
                if (curr_token.type == TOK_COLON){
                    advance_token();
                    result = (node_t){
                        .type = NOD_TERNARY_EXPRESSION,
                        .conditional_node = (nod_conditional_t){
                            .condition = make_node(result),
                            .true_block = make_node(true_val),
                            .false_block = make_node(parse_expr_level_12())
                        }
                    };
                }
                else
                    parse_error("expected ':'");
                break;
            }
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
            case TOK_ASSIGN:{
                advance_token();
                result = (node_t){
                    .type = NOD_ASSIGN,
                    .assign_node.destination = make_node(result),
                    .assign_node.source = make_node(parse_expr_level_13())
                };
                break;
            }

            case TOK_ADD_ASSIGN: {
                advance_token();
                result = (node_t){
                    .type = NOD_ASSIGN,
                    .assign_node.source = make_node((node_t){
                        .type = NOD_BINARY_ADD,
                        .binary_node.value_a = make_node(result),
                        .binary_node.value_b = make_node(parse_expr_level_13())
                    }),
                    .assign_node.destination = make_node(result)
                };
                break;
            }
            
            case TOK_SUB_ASSIGN: {
                advance_token();
                result = (node_t){
                    .type = NOD_ASSIGN,
                    .assign_node.source = make_node((node_t){
                        .type = NOD_BINARY_SUB,
                        .binary_node.value_a = make_node(result),
                        .binary_node.value_b = make_node(parse_expr_level_13())
                    }),
                    .assign_node.destination = make_node(result)
                };
                break;
            }

            case TOK_MUL_ASSIGN: {
                advance_token();
                result = (node_t){
                    .type = NOD_ASSIGN,
                    .assign_node.source = make_node((node_t){
                        .type = NOD_BINARY_MUL,
                        .binary_node.value_a = make_node(result),
                        .binary_node.value_b = make_node(parse_expr_level_13())
                    }),
                    .assign_node.destination = make_node(result)
                };
                break;
            }

            case TOK_DIV_ASSIGN: {
                advance_token();
                result = (node_t){
                    .type = NOD_ASSIGN,
                    .assign_node.source = make_node((node_t){
                        .type = NOD_BINARY_DIV,
                        .binary_node.value_a = make_node(result),
                        .binary_node.value_b = make_node(parse_expr_level_13())
                    }),
                    .assign_node.destination = make_node(result)
                };
                break;
            }

            case TOK_MOD_ASSIGN: {
                advance_token();
                result = (node_t){
                    .type = NOD_ASSIGN,
                    .assign_node.source = make_node((node_t){
                        .type = NOD_BINARY_MOD,
                        .binary_node.value_a = make_node(result),
                        .binary_node.value_b = make_node(parse_expr_level_13())
                    }),
                    .assign_node.destination = make_node(result)
                };
                break;
            }

            case TOK_XOR_ASSIGN: {
                advance_token();
                result = (node_t){
                    .type = NOD_ASSIGN,
                    .assign_node.source = make_node((node_t){
                        .type = NOD_BITWISE_XOR,
                        .binary_node.value_a = make_node(result),
                        .binary_node.value_b = make_node(parse_expr_level_13())
                    }),
                    .assign_node.destination = make_node(result)
                };
                break;
            }

            case TOK_ORR_ASSIGN: {
                advance_token();
                result = (node_t){
                    .type = NOD_ASSIGN,
                    .assign_node.source = make_node((node_t){
                        .type = NOD_BITWISE_OR,
                        .binary_node.value_a = make_node(result),
                        .binary_node.value_b = make_node(parse_expr_level_13())
                    }),
                    .assign_node.destination = make_node(result)
                };
                break;
            }

            case TOK_AND_ASSIGN: {
                advance_token();
                result = (node_t){
                    .type = NOD_ASSIGN,
                    .assign_node.source = make_node((node_t){
                        .type = NOD_BITWISE_AND,
                        .binary_node.value_a = make_node(result),
                        .binary_node.value_b = make_node(parse_expr_level_13())
                    }),
                    .assign_node.destination = make_node(result)
                };
                break;
            }

            case TOK_LSH_ASSIGN: {
                advance_token();
                result = (node_t){
                    .type = NOD_ASSIGN,
                    .assign_node.source = make_node((node_t){
                        .type = NOD_SHIFT_LEFT,
                        .binary_node.value_a = make_node(result),
                        .binary_node.value_b = make_node(parse_expr_level_13())
                    }),
                    .assign_node.destination = make_node(result)
                };
                break;
            }

            case TOK_RSH_ASSIGN: {
                advance_token();
                result = (node_t){
                    .type = NOD_ASSIGN,
                    .assign_node.source = make_node((node_t){
                        .type = NOD_SHIFT_RIGHT,
                        .binary_node.value_a = make_node(result),
                        .binary_node.value_b = make_node(parse_expr_level_13())
                    }),
                    .assign_node.destination = make_node(result)
                };
                break;
            }

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

    if (node->type)
        data_t_dstr(&node->d_type);

    switch(node->type){
        case NOD_NULL:
            return;
        case NOD_INTEGER:
            return;
        
        case NOD_RETURN:
            parse_node_dstr(node->return_node.value);
            free(node->return_node.value);
            return;
        
        case NOD_BLOCK:
            free_array(node->block_node.code, parse_node_dstr);

            return;

        case NOD_FUNC:
            free_array(node->func_node.body.code, parse_node_dstr);
            return;
        
        case NOD_POST_DECREMENT:
        case NOD_POST_INCREMENT:
        case NOD_PRE_DECREMENT:
        case NOD_PRE_INCREMENT:
        case NOD_LOGICAL_NOT: 
        case NOD_UNARY_SUB: 
        case NOD_BITWISE_NOT:
        case NOD_REFERENCE:
        case NOD_DEREFERENCE:
        case NOD_SIZEOF_EXPR:
            parse_node_dstr(node->unary_node.value);
            free(node->unary_node.value);
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
        case NOD_ADD_POINTER:
        case NOD_SUB_POINTER:
        case NOD_SUBSCRIPT:
        case NOD_SUB_POINTER_POINTER:
            parse_node_dstr(node->binary_node.value_a);
            parse_node_dstr(node->binary_node.value_b);
            free(node->binary_node.value_a);
            free(node->binary_node.value_b);
            return;

        case NOD_ASSIGN:
            parse_node_dstr(node->assign_node.source);
            parse_node_dstr(node->assign_node.destination);
            free(node->assign_node.source);
            free(node->assign_node.destination);
            return;
        
        case NOD_VARIABLE_ACCESS:
        case NOD_LONG:
        case NOD_SHORT:
        case NOD_CHAR:
        case NOD_UINTEGER:
        case NOD_USHORT:
        case NOD_ULONG:
        case NOD_UCHAR:
            return;
        
        case NOD_FUNC_CALL: {
            free_array(node->func_call_node.args, parse_node_dstr);
            return;
        }

        case NOD_TERNARY_EXPRESSION:
        case NOD_IF_STATEMENT: {
            parse_node_dstr(node->conditional_node.condition);
            parse_node_dstr(node->conditional_node.true_block);
            free(node->conditional_node.condition);
            free(node->conditional_node.true_block);
            if (node->conditional_node.false_block){
                parse_node_dstr(node->conditional_node.false_block);
                free(node->conditional_node.false_block);
            }
            return;
        }

        case NOD_DO_WHILE_LOOP:
        case NOD_WHILE_LOOP: {
            parse_node_dstr(node->while_node.code);
            parse_node_dstr(node->while_node.condition);
            
            free(node->while_node.code);
            free(node->while_node.condition);

            return;
        }

        case NOD_FOR_LOOP: {
            parse_node_dstr(node->for_node.init);
            parse_node_dstr(node->for_node.condition);
            parse_node_dstr(node->for_node.repeat);
            parse_node_dstr(node->for_node.code);
            
            free(node->for_node.init);
            free(node->for_node.condition);
            free(node->for_node.repeat);
            free(node->for_node.code);

            return;
        }

        case NOD_SWITCH_STATEMENT: {
            parse_node_dstr(node->switch_node.value);
            free_array(node->switch_node.cases, parse_node_dstr);

            free(node->switch_node.value);
            free_array(node->switch_node.code.code, parse_node_dstr);

            return;
        }

        case NOD_SWITCH_CASE:
        case NOD_BREAK:
        case NOD_CONTINUE:
        case NOD_LABEL:
        case NOD_GOTO:
        case NOD_DEFAULT_CASE:
            return;

        case NOD_STATIC_ARRAY_INIT:
        case NOD_STATIC_INIT: {
            parse_node_dstr(node->static_init_node.value);
            free(node->static_init_node.value);
            return;
        }

        case NOD_TYPE_CAST: {
            parse_node_dstr(node->type_cast_node.src);
            free(node->type_cast_node.src);

            data_t_dstr(&node->type_cast_node.dst_type);
            return;
        }

        case NOD_ARRAY_LITERAL: {
            free_array(node->array_literal_node.elems, parse_node_dstr);
            data_t_dstr(&node->array_literal_node.type);
            return;
        }

        case NOD_INIT_ARRAY: {
            parse_node_dstr(node->array_init_node.elems);
            free(node->array_init_node.elems);

            return;
        }

        case NOD_STRING_LITERAL:
            return;


        case NOD_SIZEOF_TYPE:
            data_t_dstr(&node->sizeof_type_node.d_type);
            return;
    }
}

void* array_search(void* array, void* target, bool(*comparison_func)(void* val_a, void* val_b)){
    size_t size = get_count_array(array);
    size_t elem_size = get_elem_size_array(array);
    for (size_t i = 0; i < size; ++i){
        void* elem = array + i * elem_size;
        if (comparison_func(elem, target))
            return elem;
    }
    return NULL;
}

data_type_t parse_anonymous_declaration(data_type_t d_type){    

        /* variable declaration */

        if (curr_token.type == TOK_COMMA || curr_token.type == TOK_CLOSE_PAREN)
            return d_type;

        if (curr_token.type == TOK_OPEN_BRACKET) {
            data_type_t array_type = {0};
                data_type_t* base_type = NULL;
                
                while (curr_token.type == TOK_OPEN_BRACKET){
                    advance_token();
                    if (curr_token.type == TOK_INTEGER){

                        base_type = &array_type;

                        while (base_type->ptr_derived_type)
                            base_type = base_type->ptr_derived_type;
                        
                        *base_type = (data_type_t){
                            .base_type = TYPE_ARRAY,
                            .array_size = curr_token.literals.uint,
                            .ptr_derived_type = malloc(sizeof(data_type_t))
                        };

                        *base_type->ptr_derived_type = (data_type_t){0};

                        advance_token();

                        if (curr_token.type == TOK_CLOSE_BRACKET)
                            advance_token();
                        else
                            parse_error("expected ']'");
                    }
                    else
                        parse_error("can only use signed integer constant for array subscript");
                }

                for(base_type = &array_type; base_type->base_type == TYPE_ARRAY; base_type = base_type->ptr_derived_type);

                *base_type = d_type;

                d_type = array_type;

                return d_type;
        }
        else
            parse_error("expected ',' or ')' after anonymous declaration");
        
    MARK_UNREACHABLE_CODE;
}

/*
* also handles parsing 
*/
symbol_t parse_declaration(data_type_t d_type){
    if (curr_token.type == TOK_IDENTIFIER){
        char* name = curr_token.identifier;
        advance_token();
        switch(curr_token.type){
            /* function declaration */
            case TOK_OPEN_PAREN: {
                symbol_t* parse_function_args();
                symbol_t* args = parse_function_args();
                return (symbol_t){
                    .name = name,
                    .args = args,
                    .init = false,
                    .symbol_type = SYM_FUNCTION,
                    .data_type = d_type
                };
            }

            /* variable declaration */
            case TOK_SEMICOLON:
            case TOK_COMMA:
            case TOK_CLOSE_PAREN:
            case TOK_ASSIGN: {
                return (symbol_t){
                    .name = name,
                    .init = false,
                    .symbol_type = SYM_VARIABLE,
                    .data_type = d_type
                };
            }

            case TOK_OPEN_BRACKET: {
                data_type_t array_type = {0};
                data_type_t* base_type = NULL;
                
                while (curr_token.type == TOK_OPEN_BRACKET){
                    advance_token();
                    if (curr_token.type == TOK_INTEGER){

                        base_type = &array_type;

                        while (base_type->ptr_derived_type)
                            base_type = base_type->ptr_derived_type;
                        
                        *base_type = (data_type_t){
                            .base_type = TYPE_ARRAY,
                            .array_size = curr_token.literals.uint,
                            .ptr_derived_type = malloc(sizeof(data_type_t))
                        };

                        *base_type->ptr_derived_type = (data_type_t){0};

                        advance_token();

                        if (curr_token.type == TOK_CLOSE_BRACKET)
                            advance_token();
                        else
                            parse_error("expected ']'");
                    }
                    else
                        parse_error("can only use signed integer constant for array subscript");
                }

                for(base_type = &array_type; base_type->base_type == TYPE_ARRAY; base_type = base_type->ptr_derived_type);

                *base_type = d_type;

                d_type = array_type;

                return (symbol_t){
                    .name = name,
                    .init = false,
                    .symbol_type = SYM_VARIABLE,
                    .data_type = d_type
                };
            }

            default: {
                parse_error("expected '(', ';', or '=' after declaration");
            }
        }
    }
    else
        parse_error("expected identifier after typename");

    MARK_UNREACHABLE_CODE;
}

data_type_t parse_type(){
    
    data_type_t result = {0};
    bool is_signed = true;
    bool signedness_defined = false;

    while (true){
        switch(curr_token.type){
            case KEYW_int: {
                if (!result.base_type){
                    advance_token();
                    result.base_type = TYPE_INT;
                }
                else
                    parse_error("cannot include multiple base type specifiers in one type");
                break;
            }

            case KEYW_long: {
                if (!result.base_type){
                    advance_token();
                    result.base_type = TYPE_LONG;
                }
                else
                    parse_error("cannot include multiple base type specifiers in one type");
                break;
            }

            case KEYW_static: {
                if (!result.storage_class){
                    advance_token();
                    result.storage_class = STORAGE_STATIC;
                }
                else
                    parse_error("cannot include multiple storage class specifiers in one type");
                break;
            }

            case KEYW_extern: {
                if (!result.storage_class){
                    advance_token();
                    result.storage_class = STORAGE_EXTERN;
                }
                else
                    parse_error("cannot include multiple storage class specifiers in one type");
                break;            
            }

            case KEYW_short: {
                if (!result.base_type){
                    advance_token();
                    result.base_type = TYPE_SHORT;
                }
                else
                    parse_error("cannot include multiple base type specifiers in one type");
                break;
            }

            case KEYW_char: {
                if (!result.base_type){
                    advance_token();
                    result.base_type = TYPE_CHAR;
                }
                else
                    parse_error("cannot include multiple base type specifiers in one type");
                break;
            }

            case KEYW_unsigned: {
                if (!signedness_defined){
                    is_signed = false;
                    signedness_defined = true;
                    advance_token();
                }
                else
                    parse_error("cannot specify signed-ness multiple times in one type");
                break;
            }

            case KEYW_signed: {
                if (!signedness_defined)
                    advance_token();
                else
                    parse_error("cannot specify signed-ness multiple times in one type");
                break;
            }

            case TOK_MUL: {
                if (result.base_type){
                    advance_token();
                    data_type_t ptr_base_type = result; 
                    
                    result.base_type = TYPE_POINTER;    
                    result.ptr_derived_type = malloc(sizeof(data_type_t));
                    *result.ptr_derived_type = ptr_base_type;
                }
                else
                    return (data_type_t){0};

                break;
            }

            case KEYW_void: {
                if (!result.base_type){
                    if (!signedness_defined){
                        advance_token();
                        result.base_type = TYPE_VOID;
                        break;
                    }
                    else
                        parse_error("stray 'void' in declaration");
                }
                else
                    parse_error("stray 'void' in declaration");
            }

            default: {
                if (!is_signed)
                    /* turns the base type to its unsigned counterpart */
                    result.base_type += 5;
                return result;
            }
        }
    }
    MARK_UNREACHABLE_CODE;
}

/* 
* assumes that curr_token.type == TOK_OPEN_PAREN 
* does not parse function args without name
*/
symbol_t* parse_function_args(){
    symbol_t* args = alloc_array(sizeof(symbol_t), 1);
    symbol_t var;

    if (curr_token.type == TOK_OPEN_PAREN){
        advance_token();
        
        for (data_type_t d_type = parse_type(); d_type.base_type || curr_token.type == TOK_ELIPSES; d_type = parse_type()){
            if (d_type.base_type){
                var = parse_declaration(d_type);
                pback_array(&args, &var);
            } else
            if (curr_token.type == TOK_ELIPSES){
                var = (symbol_t){
                    .symbol_type = SYM_VARIADIC
                };
                pback_array(&args, &var);

                advance_token();
            }
            else
                parse_error("expected typename or '...'");

            if (curr_token.type == TOK_COMMA)
                advance_token(); else
            if (curr_token.type == TOK_CLOSE_PAREN)
                break;
            else
                parse_error("expected ','");
        }

        if (curr_token.type == TOK_CLOSE_PAREN)
            advance_token();
        else
            parse_error("expected ')'");
    }
    else
        parse_error("expected '('");

    return args;
}   

node_t parse_array_literal(data_type_t type){
    data_type_t clone_data_t(data_type_t src);
    
    node_t node;

    node_t* elems = alloc_array(sizeof(node_t), 1);

    if (curr_token.type == TOK_OPEN_BRACE){
        advance_token();
        while (curr_token.type != TOK_CLOSE_BRACE){
            if (curr_token.type == TOK_OPEN_BRACE){
                node = parse_array_literal(*type.ptr_derived_type);

                node.array_literal_node.type = clone_data_t(*type.ptr_derived_type);

                pback_array(&elems, &node);

                advance_token();
                if (curr_token.type == TOK_COMMA)
                    advance_token();
            }
            else {
                node = parse_expr();
                pback_array(&elems, &node);

                if (curr_token.type == TOK_COMMA)
                    advance_token();
            }
        }
    }
    else
        parse_error("expected '}'");
    
    return (node_t){
        .type = NOD_ARRAY_LITERAL,
        .array_literal_node.elems = elems,
        .array_literal_node.type = clone_data_t(*type.ptr_derived_type)
    };
}