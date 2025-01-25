#include <stdlib.h>
#include <stdio.h>

#include "typecheck.h"
#include "../lib/vector/C_vector.h"

void typecheck_error(const char* msg)  {
    fprintf(stderr, "type error: %s\n", msg);
    exit(-1);
}

void typecheck(node_t* nodes){
    void typecheck_node(node_t* node);

    for (size_t i = 0; i < get_count_array(nodes); ++i)
        typecheck_node(nodes + i);
}

extern scope_t* global_scope;
/*
    check_storage_class:
        checks if the storage class between two types is equivalent
    array_pointer_decay:
        if (type_1 is array && type_2 is pointer) && (base types match) -> true
        -> whats annoying about this is that it sets a specific order for this function
    commutative:
        array_pointer_decay but in reverse as well
*/
bool type_comp(data_type_t* type_1, data_type_t* type_2, bool check_storage_class, bool array_pointer_decay, bool commutative){
    if (type_1->base_type == TYPE_POINTER && type_2->base_type == TYPE_POINTER)
        return type_comp(type_1->ptr_derived_type, type_2->ptr_derived_type, check_storage_class, array_pointer_decay, commutative); else
    if (type_1->base_type == TYPE_ARRAY && type_2->base_type == TYPE_ARRAY)
        return type_1->array_size == type_2->array_size && type_comp(type_1->ptr_derived_type, type_2->ptr_derived_type, check_storage_class, array_pointer_decay, commutative); else
    if ((array_pointer_decay && type_1->base_type == TYPE_ARRAY && type_2->base_type == TYPE_POINTER) || (commutative && array_pointer_decay && type_1->base_type == TYPE_POINTER && type_2->base_type == TYPE_ARRAY)){
        type_2->base_type = type_1->base_type = TYPE_POINTER;
        return type_comp(type_1->ptr_derived_type, type_2->ptr_derived_type, check_storage_class, array_pointer_decay, commutative);
    }
    else
        return (type_1)->base_type == (type_2)->base_type && (!(check_storage_class) || (type_1)->storage_class == (type_2)->storage_class);
}

#define is_constant(node_type)          \
    ((node_type == NOD_INTEGER)      || \
    (node_type == NOD_LONG)          || \
    (node_type == NOD_SHORT)         || \
    (node_type == NOD_CHAR)          || \
    (node_type == NOD_UINTEGER)      || \
    (node_type == NOD_ULONG)         || \
    (node_type == NOD_USHORT)        || \
    (node_type == NOD_UCHAR))

data_type_t clone_data_t(data_type_t src){
    if (src.base_type == TYPE_ARRAY || src.base_type == TYPE_POINTER){
        data_type_t result = (data_type_t){
            .base_type = src.base_type,
            .array_size = src.array_size,
            .storage_class = src.storage_class,
            .ptr_derived_type = malloc(sizeof(data_type_t))
        };

        *result.ptr_derived_type = clone_data_t(*src.ptr_derived_type);
        
        return result;
    }
    else
        return src;
}

#define swap_pointer(p_1, p_2)     \
    {                              \
        void* intermediate = (p_1);\
        (p_1) = (p_2);             \
        (p_2) = intermediate;      \
    }

void typecheck_node(node_t* node){
    static symbol_t* curr_func;
    static symbol_t** curr_scope;
    /* "symbol_t" related functions defined in "../parser/parser.c" that i want to use here */
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

            if (type_comp(&curr_func->data_type, &node->return_node.value->d_type, false, false, false))
                node->d_type = clone_data_t(node->return_node.value->d_type);
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

            if (func->data_type.base_type == TYPE_ARRAY)
                typecheck_error("function cannot return array type");

            curr_func = func;
            curr_scope = node->func_node.function_scope;

            for (size_t i = 0; i < get_count_array(func->args); ++i)
                if (func->args[i].data_type.base_type == TYPE_ARRAY)
                    func->args[i].data_type.base_type = TYPE_POINTER;
            
            node_t* block = make_node((node_t){.block_node = node->func_node.body, .type = NOD_BLOCK});
            typecheck_node(block);
            free(block);

            node->d_type = (data_type_t){0};

            return;
        }

        case NOD_FUNC_CALL: {

            symbol_t* func = find_symbol(global_scope, (symbol_t){
                .name = node->func_call_node.identifier,
                .symbol_type = SYM_FUNCTION
            }, NULL, NULL);


            if (func){
                for (size_t i = 0; i < get_count_array(node->func_call_node.args); ++i){
                    typecheck_node(node->func_call_node.args + i);

                    if (!type_comp(&node->func_call_node.args[i].d_type, &func->args[i].data_type, false, true, false))
                        typecheck_error("mismatching function argument types");
                }

                if (get_count_array(node->func_call_node.args) == get_count_array(func->args))
                    node->d_type = clone_data_t(func->data_type);
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

            node->d_type = clone_data_t(node->unary_node.value->d_type);

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

            /* pointer addition */
            if (node->type == NOD_BINARY_ADD && (node->binary_node.value_a->d_type.base_type == TYPE_POINTER || node->binary_node.value_b->d_type.base_type == TYPE_POINTER)){
                
                if (node->binary_node.value_a->d_type.base_type == TYPE_UNSIGNED_LONG && node->binary_node.value_b->d_type.base_type == TYPE_POINTER){
                    swap_pointer(node->binary_node.value_a, node->binary_node.value_b);
                }

                node->d_type = clone_data_t(node->binary_node.value_a->d_type);
                node->type = NOD_ADD_POINTER;
    
            } else
            if (node->type == NOD_BINARY_SUB && (node->binary_node.value_a->d_type.base_type == TYPE_POINTER || node->binary_node.value_b->d_type.base_type == TYPE_POINTER)){
                
                if (node->binary_node.value_a->d_type.base_type == TYPE_UNSIGNED_LONG && node->binary_node.value_b->d_type.base_type == TYPE_POINTER){
                    swap_pointer(node->binary_node.value_a, node->binary_node.value_b);
                }

                node->d_type = clone_data_t(node->binary_node.value_a->d_type);
                node->type = NOD_SUB_POINTER;
    
            }
            else {
                if (type_comp(&node->binary_node.value_a->d_type, &node->binary_node.value_b->d_type, false, true, true)){
                    node->d_type = clone_data_t(node->binary_node.value_a->d_type);
                }
                else
                    typecheck_error("attempt to use two different types in binary operation");
            }
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

            if (type_comp(&node->binary_node.value_a->d_type, &node->binary_node.value_b->d_type, false, true, true)){
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

            if (!type_comp(&node->conditional_node.true_block->d_type, &node->conditional_node.false_block->d_type, false, false, true))
                typecheck_error("cannot return two different types in ternary expression");
            
            node->d_type = clone_data_t(node->conditional_node.true_block->d_type);

            return;
        }

        case NOD_STATIC_INIT: {
            typecheck_node(node->static_init_node.value);

            if (!type_comp(&node->static_init_node.value->d_type, &node->static_init_node.sym_info.data_type, false, false, false))
                typecheck_error("cannot assign type B to type A");

            if (!is_constant(node->static_init_node.value->type))
                typecheck_error("cannot assign non-constant expression in static variable initializer");

            node->d_type = (data_type_t){0};
            return;
        }

        case NOD_TYPE_CAST: {
            typecheck_node(node->type_cast_node.src);

            if (node->type_cast_node.dst_type.base_type == TYPE_ARRAY)
                typecheck_error("cannot cast to array type");
            else
                node->d_type = clone_data_t(node->type_cast_node.dst_type);

            return;
        }

        case NOD_VARIABLE_ACCESS: {
            node->d_type = clone_data_t(curr_scope[node->var_access_node.var_info.scope_id][node->var_access_node.var_info.var_num].data_type);

            return;
        }

        case NOD_ASSIGN: {
            typecheck_node(node->assign_node.source);
            typecheck_node(node->assign_node.destination);

            if (type_comp(&node->assign_node.source->d_type, &node->assign_node.destination->d_type, false, true, false))
                node->d_type = clone_data_t(node->assign_node.source->d_type);
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
            node->d_type = (data_type_t){0};
            return;
        }

        case NOD_REFERENCE: {
            typecheck_node(node->unary_node.value);

            data_type_t* derived_type = malloc(sizeof(data_type_t));

            node->d_type.base_type = TYPE_POINTER;

            if (node->unary_node.value->d_type.base_type == TYPE_ARRAY)
                *derived_type = clone_data_t(*node->unary_node.value->d_type.ptr_derived_type);
            else
                *derived_type = clone_data_t(node->unary_node.value->d_type);

            node->d_type.ptr_derived_type = derived_type;

            return;
        }

        case NOD_DEREFERENCE: {
            typecheck_node(node->unary_node.value);

            if (node->unary_node.value->d_type.base_type == TYPE_POINTER || node->unary_node.value->d_type.base_type == TYPE_ARRAY)
                node->d_type = clone_data_t(*node->unary_node.value->d_type.ptr_derived_type);
            else
                typecheck_error("cannot dereference non-pointer object");
            return;
        }

        case NOD_ARRAY_LITERAL: {
            for (size_t i = 0; i < get_count_array(node->array_literal_node.elems); ++i){
                typecheck_node(node->array_literal_node.elems + i);
                if (!type_comp(&node->array_literal_node.elems[i].d_type, node->array_literal_node.type.ptr_derived_type, false, false, false))
                    typecheck_error("array literal element type mismatch");
            }

            node->d_type = clone_data_t(node->array_literal_node.type);

            return;
        }

        case NOD_INIT_ARRAY: {
            typecheck_node(node->array_init_node.elems);

            if (node->array_init_node.elems->d_type.base_type == TYPE_ARRAY)
                if (type_comp(&curr_scope[node->array_init_node.array_var.scope_id][node->array_init_node.array_var.var_num].data_type, &node->array_init_node.elems->d_type, false, false, false))
                    node->d_type = (data_type_t){0};
                else
                    typecheck_error("mismatch in array object and array variable type");
            else
                typecheck_error("cannot initialize array object with non-array type");
            
            return;
        }

        case NOD_SUBSCRIPT: {
            node_t *make_node(node_t node);
            
            typecheck_node(node->binary_node.value_a);
            typecheck_node(node->binary_node.value_b);

            if (node->binary_node.value_a->d_type.base_type == TYPE_ARRAY || node->binary_node.value_a->d_type.base_type == TYPE_POINTER){
                if (node->binary_node.value_b->d_type.base_type != TYPE_UNSIGNED_LONG)
                    typecheck_error("can only subscript array / pointer type and unsigned long");
            } else
            if (node->binary_node.value_b->d_type.base_type == TYPE_ARRAY || node->binary_node.value_b->d_type.base_type == TYPE_POINTER){
                if (node->binary_node.value_a->d_type.base_type == TYPE_UNSIGNED_LONG)
                    swap_pointer(node->binary_node.value_a, node->binary_node.value_b) /* missing semicolon because macro ends in a '}' */
                else
                    typecheck_error("can only subscript array / pointer type and unsigned long");
            }
            else
                typecheck_error("one operand of subscript must be pointer / array type");

            if (node->binary_node.value_a->d_type.base_type == TYPE_ARRAY){
                *node->binary_node.value_a = (node_t){
                    .type = NOD_REFERENCE,
                    .unary_node.value = make_node(*node->binary_node.value_a)
                };

                typecheck_node(node->binary_node.value_a);
            }

            node->type = NOD_ADD_POINTER;

            node->d_type = clone_data_t(node->binary_node.value_a->d_type);
            
            *node = (node_t){
                .type = NOD_DEREFERENCE,
                .unary_node.value = make_node(*node)
            };

            node->d_type = clone_data_t(*node->unary_node.value->binary_node.value_a->d_type.ptr_derived_type);

            return;
        }

        /* these are already typechecked, as they are implemented by the typechecker themselves */
        case NOD_ADD_POINTER:
        case NOD_SUB_POINTER:
            return;

        default: {
            #define __STRINGIFY(x) #x
            #define __TO_STRING(x) __STRINGIFY(x)

            typecheck_error("exception: unknown parse node: typecheck.c:" __TO_STRING(__LINE__));
        }
    }
}