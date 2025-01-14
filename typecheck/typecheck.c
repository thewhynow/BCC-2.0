#include <stdlib.h>
#include <stdio.h>

#include "typecheck.h"
#include "../lib/vector/C_vector.h"

void typecheck_error(const char* msg){
    fprintf(stderr, "type error: %s\n", msg);
    exit(-1);
}

void typecheck(node_t* nodes){
    void typecheck_node(node_t* node);

    for (size_t i = 0; i < get_count_array(nodes); ++i)
        typecheck_node(nodes + i);
}

extern scope_t* global_scope;

#define type_comp(type_1, type_2, check_storage_class)\
    ((type_1).base_type == (type_2).base_type && (!(check_storage_class) || (type_1).storage_class == (type_2).storage_class))

#define is_constant(node_type)     \
    ((node_type == NOD_INTEGER) || \
    (node_type == NOD_LONG)     || \
    (node_type == NOD_SHORT)    || \
    (node_type == NOD_CHAR)     || \
    (node_type == NOD_UINTEGER) || \
    (node_type == NOD_ULONG)    || \
    (node_type == NOD_USHORT)   || \
    (node_type == NOD_UCHAR))

void typecheck_node(node_t* node){
    static symbol_t* curr_func;
    static symbol_t** curr_scope;
    /* "symbol_t" related funcitons defined in "../parser/parser.c" that i want to use here */
    bool symbol_comp(void* _symbol_a, void* _symbol_b);
    symbol_t* find_symbol(scope_t* scope, symbol_t target, size_t* scope_id_out, size_t* var_num_out);


    data_type_t get_d_type(node_t* node);

    switch(node->type){
        case NOD_NULL: {
            node->d_type = (data_type_t){0};
            return;
        }

        case NOD_INTEGER: {
            node->d_type = (data_type_t){
                .base_type = TYPE_INT,
                .storage_class = STORAGE_NULL
            };
            return;
        }

        case NOD_LONG: {
            node->d_type = (data_type_t){
                .base_type = TYPE_LONG,
                .storage_class = STORAGE_NULL
            };
            return;
        }

        case NOD_SHORT: {
            node->d_type = (data_type_t){
                .base_type = TYPE_SHORT,
                .storage_class = STORAGE_NULL
            };
            return;
        }

        case NOD_CHAR: {
            node->d_type = (data_type_t){
                .base_type = TYPE_CHAR,
                .storage_class = STORAGE_NULL
            };
            return;
        }

        case NOD_RETURN: {
            typecheck_node(node->return_node.value);

            if (type_comp(curr_func->data_type, node->return_node.value->d_type, false))
                node->d_type = node->return_node.value->d_type;
            else
                typecheck_error("return statement has mismatching type with function declaration");

            return;
        }

        case NOD_FUNC: {
            node_t* make_node(node_t node);

            symbol_t* func = find_symbol(global_scope, (symbol_t){
                .name = node->func_node.name,
                .data_type = node->func_node.type,
                .symbol_type = SYM_FUNCTION,
            }, NULL, NULL);

            curr_func = func;
            curr_scope = node->func_node.function_scope;
            
            node_t* block = make_node((node_t){.block_node = node->func_node.body, .type = NOD_BLOCK});
            typecheck_node(block);
            free(block);

            return;
        }

        case NOD_FUNC_CALL: {

            symbol_t* func = find_symbol(global_scope, (symbol_t){
                .name = node->func_call_node.identifier,
                .symbol_type = SYM_FUNCTION
            }, NULL, NULL);
            
            for (size_t i = 0; i < get_count_array(node->func_call_node.args); ++i){
                typecheck_node(node->func_call_node.args + i);

                if (!type_comp(node->func_call_node.args[i].d_type, func->args[i].data_type, false))
                    typecheck_error("mismatching function argument types");
            }

            if (func){
                if (get_count_array(node->func_call_node.args) == get_count_array(func->args))
                    node->d_type = func->data_type;
                else
                    typecheck_error("invalid number of arguments passed to function");
            }
            else
                typecheck_error("attempt to call undefined function");

            return;
        }

        case NOD_LOGICAL_NOT: {
            typecheck_node(node->unary_node.value);

            node->d_type = (data_type_t){
                .base_type = TYPE_INT,
                .storage_class = STORAGE_NULL,
            };

            return;
        }

        case NOD_PRE_INCREMENT:
        case NOD_POST_INCREMENT:
        case NOD_PRE_DECREMENT:
        case NOD_POST_DECREMENT:
        case NOD_BITWISE_NOT:
        case NOD_UNARY_SUB: {
            typecheck_node(node->unary_node.value);

            node->d_type = node->unary_node.value->d_type;

            return;
        }

        case NOD_BINARY_ADD:
        case NOD_BINARY_SUB:
        case NOD_BINARY_DIV:
        case NOD_BINARY_MUL:
        case NOD_BINARY_MOD:
        case NOD_BITWISE_AND:
        case NOD_BITWISE_OR:
        case NOD_BITWISE_XOR:
        case NOD_SHIFT_LEFT:
        case NOD_SHIFT_RIGHT:{
            typecheck_node(node->binary_node.value_a);
            typecheck_node(node->binary_node.value_b);

            if (type_comp(node->binary_node.value_a->d_type, node->binary_node.value_b->d_type, false)){
                node->d_type = node->binary_node.value_a->d_type;
            }
            else
                typecheck_error("attempt to use two different types in binary operation");
            
            return;
        }

        case NOD_LOGICAL_AND:
        case NOD_LOGICAL_OR:
        case NOD_EQUAL_TO:
        case NOD_NOT_EQUAL_TO:
        case NOD_LESS_THAN:
        case NOD_GREATER_THAN:
        case NOD_LESS_THAN_OR_EQUAL:
        case NOD_GREATER_THAN_OR_EQUAL: {
            typecheck_node(node->binary_node.value_a);
            typecheck_node(node->binary_node.value_b);

            if (type_comp(node->binary_node.value_a->d_type, node->binary_node.value_b->d_type, false)){
                node->d_type = (data_type_t){
                    .base_type = TYPE_INT,
                    .storage_class = STORAGE_NULL
                };
            }
            else
                typecheck_error("attempt to use two different types in logical comparison");
            
            return;
        }

        case NOD_TERNARY_EXPRESSION: {
            typecheck_node(node->conditional_node.condition);
            typecheck_node(node->conditional_node.true_block);
            typecheck_node(node->conditional_node.false_block);

            if (!type_comp(node->conditional_node.true_block->d_type, node->conditional_node.true_block->d_type, false))
                typecheck_error("cannot return two different types in ternary expression");
        }

        case NOD_STATIC_INIT: {
            typecheck_node(node->static_init_node.value);

            if (!type_comp(node->static_init_node.value->d_type, node->static_init_node.sym_info.data_type, false))
                typecheck_error("cannot assign type B to type A");

            if (!is_constant(node->static_init_node.value->type))
                typecheck_error("cannot assign non-constant expression in static variable initializer");

            node->d_type = (data_type_t){0};
            return;
        }

        case NOD_TYPE_CAST: {
            typecheck_node(node->type_cast_node.src);
            node->d_type = node->type_cast_node.dst_type;
            return;
        }

        case NOD_VARIABLE_ACCESS: {
            node->d_type = curr_scope[node->var_access_node.var_info.scope_id][node->var_access_node.var_info.var_num].data_type;

            return;
        }

        case NOD_ASSIGN: {
            typecheck_node(node->assign_node.source);
            typecheck_node(node->assign_node.destination);

            if (type_comp(node->assign_node.source->d_type, node->assign_node.destination->d_type, false))
                node->d_type = node->assign_node.source->d_type;
            else
                typecheck_error("cannot assign expression of type A to expression of type B");

            return;
        }

        case NOD_IF_STATEMENT: {
            typecheck_node(node->conditional_node.condition);
            typecheck_node(node->conditional_node.true_block);
            if (node->conditional_node.false_block)
                typecheck_node(node->conditional_node.false_block);
            node->d_type = (data_type_t){0};
            return;
        }

        case NOD_WHILE_LOOP:
        case NOD_DO_WHILE_LOOP: {
            typecheck_node(node->while_node.condition);
            typecheck_node(node->while_node.code);
            node->d_type = (data_type_t){0};
            return;
        }

        case NOD_BLOCK: {
            for (size_t i = 0; i < get_count_array(node->block_node.code); ++i)
                typecheck_node(i + node->block_node.code);
            node->d_type = (data_type_t){0};
            return;
        }

        case NOD_SWITCH_STATEMENT: {
            node_t* make_node(node_t node);
            typecheck_node(node->switch_node.value);
            node_t* block = make_node((node_t){.block_node = node->switch_node.code, .type = NOD_BLOCK});
            typecheck_node(block);
            free(block);
            node->d_type = (data_type_t){0};
            return;
        }

        case NOD_SWITCH_CASE: {
            typecheck_node(node->unary_node.value);
            node->d_type = (data_type_t){0};
            return;
        }

        case NOD_DEFAULT_CASE:
        case NOD_CONTINUE:
        case NOD_GOTO:
        case NOD_BREAK:
        case NOD_LABEL: {
            return;
        }

        case NOD_UINTEGER: {
            node->d_type = (data_type_t){
                .base_type = TYPE_UNSIGNED_INT,
                .storage_class = STORAGE_NULL
            };
            return;
        }

        case NOD_ULONG: {
            node->d_type = (data_type_t){
                .base_type = TYPE_UNSIGNED_LONG,
                .storage_class = STORAGE_NULL
            };
            return;
        }

        case NOD_USHORT: {
            node->d_type = (data_type_t){
                .base_type = TYPE_UNSIGNED_SHORT,
                .storage_class = STORAGE_NULL
            };
            return;
        }

        case NOD_UCHAR: {
            node->d_type = (data_type_t){
                .base_type = TYPE_UNSIGNED_CHAR,
                .storage_class = STORAGE_NULL
            };
            return;
        }

        case NOD_FOR_LOOP: {
            typecheck_node(node->for_node.init);
            typecheck_node(node->for_node.condition);
            typecheck_node(node->for_node.repeat);
            typecheck_node(node->for_node.code);
            return;
        }
    }
}