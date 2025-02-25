#include "ir_gen.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

size_t frame_size;

static void ir_gen_func(nod_function_t node);
static ir_node_t* ir_code;

static symbol_t** function_scope;
extern scope_t* global_scope;

static ir_operand_t* global_vars;
static ir_operand_t** all_vars;
static ir_operand_t* curr_vars;

static size_t get_type_size(data_type_t type);

static void ir_gen_node(node_t node);

void symbol_dstr(void* _symbol);

static size_t* DEBUG_THING; 

#define var_gen(size)                      \
    (frame_size += (size), (ir_operand_t){ \
        .type = STK_OFFSET,                \
        .offset = -(signed long)frame_size \
    })

#define is_unsigned(type) ((__UNSIGNED_TYPE_START__ < (type).base_type && (type).base_type < __UNSIGNED_TYPE_END__) || (type).base_type == TYPE_POINTER)

ir_node_t* read_only_initializers;
ir_node_t* static_initializers;
ir_node_t* zero_initializers;

#define is_arg_register(value)\
    ((value).type == REG_DI || (value).type == REG_SI || (value).type == REG_DX || (value).type == REG_CX || (value).type == REG_R8 || (value).type == REG_R9)

static size_t func_num = 0LU;
char** func_names;

ir_operand_t get_val_offset(ir_operand_t val, long offset);

ir_node_t* ir_gen(node_t* program_ast){
    global_vars = alloc_array(sizeof(ir_operand_t), 1);
    ir_code = alloc_array(sizeof(ir_node_t), 1);
    read_only_initializers = alloc_array(sizeof(ir_node_t), 1);
    static_initializers = alloc_array(sizeof(ir_node_t), 1);
    zero_initializers = alloc_array(sizeof(ir_node_t), 1);
    func_names = alloc_array(sizeof(char*), 1);

    for (size_t i = 0; i < get_count_array(global_scope->top_scope); ++i){
        ir_operand_t value;
        if (global_scope->top_scope[i].symbol_type == SYM_FUNCTION){
                value = (ir_operand_t){
                    .type = FUNCTION,
                    .identifier = global_scope->top_scope[i].name,
                };
                pback_array(&global_vars, &value);
        } else
        if (global_scope->top_scope[i].symbol_type == SYM_VARIABLE){
                value = (ir_operand_t){
                    .type = STATIC_MEM,
                    .identifier = global_scope->top_scope[i].name,
                };

                if (!global_scope->top_scope[i].init && global_scope->top_scope[i].data_type.storage_class != STORAGE_EXTERN){
                    ir_node_t ir_node = (ir_node_t){
                        .instruction = INST_INIT_STATIC_Z,
                        .dst.identifier = value.identifier,
                        .op_2.label_id = get_type_size(global_scope->top_scope[i].data_type),
                    };
                    pback_array(&zero_initializers, &ir_node);
                }
                pback_array(&global_vars, &value);
        }
        else {
            value = (ir_operand_t){
                .type = OP_NULL,
            };
            pback_array(&global_vars, &value);
        }
    }

    size_t count = get_count_array(ir_code);
    for (size_t i = 0; i < get_count_array(program_ast); ++i){
        ir_gen_node(program_ast[i]);
    }

    free_array(global_vars, NULL);

    return ir_code;
}

static void ir_gen_func(nod_function_t node){
    ir_operand_t ir_gen_function_arg(size_t arg_num);
    frame_size = 0;
    function_scope = node.function_scope;

    pback_array(&func_names, &node.name);

    all_vars = alloc_array(sizeof(ir_operand_t*), 1);
    pback_array(&all_vars, &global_vars);
    curr_vars = alloc_array(sizeof(ir_operand_t), 1);

    function_scope[0] = global_scope->top_scope;

    ir_node_t ir_node = (ir_node_t){
        .dst.identifier = node.name,
        .op_1.label_id = frame_size
    };
    if (node.type.storage_class == STORAGE_STATIC)
        ir_node.instruction = INST_STATIC_FUNC;
    else
        ir_node.instruction = INST_PUBLIC_FUNC;
    pback_array(&ir_code, &ir_node);
    
    size_t func_node_index = get_count_array(ir_code) - 1;
    
    size_t i = 0;

    ir_operand_t* generate_func_args(symbol_t* sym_args, size_t return_size);

    ir_operand_t* val_args = generate_func_args(node.args, get_type_size(node.type));

    for (; i < get_count_array(val_args); ++i)
        pback_array(&curr_vars, val_args + i);

    free_array(val_args, NULL);

    {
        ir_operand_t variable;
        for (; i < get_count_array(function_scope[node.body.scope_id]); ++i){
            if (function_scope[node.body.scope_id][i].symbol_type == SYM_FUNCTION)
                variable = (ir_operand_t){
                    .type = FUNCTION,
                    .identifier = function_scope[node.body.scope_id][i].name,
                };
            else
            if (function_scope[node.body.scope_id][i].data_type.storage_class == STORAGE_STATIC){
                variable = (ir_operand_t){
                    .type = STATIC_MEM_LOCAL,
                    .identifier = function_scope[node.body.scope_id][i].name,
                };

                if (!function_scope[node.body.scope_id][i].init){
                    ir_node = (ir_node_t){
                        .instruction = INST_INIT_STATIC_Z_LOCAL,
                        .dst.identifier = function_scope[node.body.scope_id][i].name,
                        .op_2.label_id = get_type_size(function_scope[node.body.scope_id][i].data_type),
                        .size = func_num
                    };
                    pback_array(&zero_initializers, &ir_node);
                }
            } else
            if (function_scope[node.body.scope_id][i].data_type.storage_class == STORAGE_EXTERN)
                variable = (ir_operand_t){
                    .type = STATIC_MEM,
                    .identifier = function_scope[node.body.scope_id][i].name,
                };
            else{
                frame_size += get_type_size(function_scope[node.body.scope_id][i].data_type);

                variable = (ir_operand_t){
                    .type = STK_OFFSET,
                    .offset = - frame_size
                };
            }
            pback_array(&curr_vars, &variable);
        }

        pback_array(&all_vars, &curr_vars);

        size_t code_len = get_count_array(node.body.code);
        for (size_t i = 0; i < code_len; ++i)
            ir_gen_node(node.body.code[i]);
    }

    /* implicit return 0 in all functions */
    ir_node = (ir_node_t){
        .instruction = INST_COPY,
        .op_1 = (ir_operand_t){
            .type = IMM_U32,
            .immediate = 0
        },
        .dst = (ir_operand_t){.type = REG_AX},
        .size = 4
    };
    pback_array(&ir_code, &ir_node);
    
    ir_node = (ir_node_t){
        .instruction = INST_RET,
    };
    pback_array(&ir_code, &ir_node);

    /* ensure stack is 16-byte aligned */
    if (frame_size != 0)
        frame_size += 16 - (frame_size % 16);

    ir_code[func_node_index].op_1.label_id = frame_size;

    for (size_t i = 1; i < get_count_array(all_vars); ++i)
        free_array(all_vars[i], NULL);
    free_array(all_vars, NULL);

    for (size_t i = 1; i < get_count_array(function_scope); ++i)
        free_array(function_scope[i], symbol_dstr);
    free_array(function_scope, NULL);

    func_num++;
}

static ir_operand_t value = {0};

void ir_gen_binary_op(ir_inst_t inst, node_t* a, node_t* b){
    ir_gen_node(*a);

    ir_node_t ir_node = (ir_node_t){
        .instruction = INST_COPY,
        .op_1 = value,
        .dst = var_gen(get_type_size(a->d_type)),
        .size = get_type_size(a->d_type)
    };
    pback_array(&ir_code, &ir_node);

    ir_operand_t val_a = ir_node.dst;

    ir_gen_node(*b);

    ir_node = (ir_node_t){
        .instruction = inst,
        .op_1 = val_a,
        .op_2 = value,
        .dst = val_a,
        .size = get_type_size(a->d_type)
    };
    pback_array(&ir_code, &ir_node);

    value = ir_node.dst;
}

void ir_gen_binary_comp(ir_inst_t inst, node_t* a, node_t* b){
    ir_gen_node(*a);
    ir_operand_t val_a = value;

    ir_gen_node(*b);

    if (is_unsigned(a->d_type)){
        if (INST_SET_G <= inst && inst <= INST_SET_LE)
            inst += INST_SET_B - INST_SET_L;
    }

    ir_node_t ir_node = (ir_node_t){
        .instruction = inst,
        .op_1 = val_a,
        .op_2 = value,
        .dst = var_gen(4),
        .size = get_type_size(a->d_type)
    };
    pback_array(&ir_code, &ir_node);

    value = ir_node.dst;
}

static size_t get_type_size(data_type_t type);

void ir_gen_init_compound_literal(node_t *literal, ir_operand_t start);
void ir_gen_static_compound_literal(node_t* literal);

static void ir_gen_node(node_t node){
    static ir_node_t ir_node = (ir_node_t){.instruction = INST_NOP};
    /* all labels are 'L%lu' */
    static size_t label_count = 0;
    static size_t break_label = 0;
    static size_t continue_label = 0;
    static size_t default_case_label = 0;
    static size_t inst_size = 0;
    static size_t old_label_count = 0;

    switch(node.type){
        case NOD_NULL:{
            value = (ir_operand_t){
                .type = IMM_U8,
                .immediate = 0
            };
            return;
        }

        case NOD_UINTEGER:
        case NOD_INTEGER: {
            value = (ir_operand_t){
                .type = IMM_U32,
                .immediate = node.const_node.int_literal,
            };
            return;
        }

        case NOD_ULONG:
        case NOD_LONG: {
            value = (ir_operand_t){
                .type = IMM_U64,
                .immediate = node.const_node.long_literal
            };
            return;
        }

        case NOD_USHORT:
        case NOD_SHORT: {
            value = (ir_operand_t){
                .type = IMM_U16,
                .immediate = node.const_node.short_literal
            };
            return;
        }

        case NOD_UCHAR:
        case NOD_CHAR: {
            value = (ir_operand_t){
                .type = IMM_U8,
                .immediate = node.const_node.char_literal
            };
            return;
        }

        case NOD_RETURN: {
            ir_gen_node(*node.return_node.value);

            size_t t_size = get_type_size(node.return_node.value->d_type);

            if (t_size > 16){
                ir_node = (ir_node_t){
                    .instruction = INST_COPY,
                    .op_1.type = REG_DI,
                    .dst.type = REG_R11,
                    .size = 8
                };
                pback_array(&ir_code, &ir_node);

                ir_node = (ir_node_t){
                    .instruction = INST_COPY,
                    .op_1 = value,
                    .dst.type = MEM_ADDRESS,
                    .size = t_size
                };
                pback_array(&ir_code, &ir_node);
            } else
            if (t_size > 8){
                ir_node = (ir_node_t){
                    .instruction = INST_COPY,
                    .op_1 = value,
                    .dst.type = REG_AX,
                    .size = 8
                };
                pback_array(&ir_code, &ir_node);
                
                ir_node = (ir_node_t){
                    .instruction = INST_COPY,
                    .op_1 = get_val_offset(value, 8),
                    .dst.type = REG_DX,
                    .size = 8
                };
                pback_array(&ir_code, &ir_node);
            }
            else {
                ir_node = (ir_node_t){
                    .instruction = INST_COPY,
                    .op_1 = value,
                    .dst.type = REG_AX,
                    .size = t_size
                };
                pback_array(&ir_code, &ir_node);
            }

            ir_node.instruction = INST_RET;
            pback_array(&ir_code, &ir_node);

            return;
        }

        case NOD_BLOCK: {
            ir_operand_t* old_vars = curr_vars;
            curr_vars = alloc_array(sizeof(ir_operand_t), 1);

            ir_operand_t variable;

            size_t var_amount = get_count_array(function_scope[node.block_node.scope_id]);
            for (size_t i = 0; i < var_amount; ++i){
                if (function_scope[node.block_node.scope_id][i].data_type.storage_class == STORAGE_STATIC){
                    variable = (ir_operand_t){
                        .type = STATIC_MEM_LOCAL,
                        .identifier = function_scope[node.block_node.scope_id][i].name,
                    };

                    if (!function_scope[node.block_node.scope_id][i].init){
                        ir_node = (ir_node_t){
                            .instruction = INST_INIT_STATIC_Z_LOCAL,
                            .dst.identifier = function_scope[node.block_node.scope_id][i].name,
                            .op_2.label_id = get_type_size(function_scope[node.block_node.scope_id][i].data_type),
                            .size = func_num
                        };
                        pback_array(&zero_initializers, &ir_node);
                    }
                } else
                if (function_scope[node.block_node.scope_id][i].data_type.storage_class == STORAGE_EXTERN)
                    variable = (ir_operand_t){
                        .type = STATIC_MEM,
                        .identifier = function_scope[node.block_node.scope_id][i].name,
                    };
                else{
                    frame_size += get_type_size(function_scope[node.block_node.scope_id][i].data_type);

                    variable = (ir_operand_t){
                        .type = STK_OFFSET,
                        .offset = -frame_size
                    };

                }
                pback_array(&curr_vars, &variable);
            }

            pback_array(&all_vars, &curr_vars);

            size_t code_len = get_count_array(node.block_node.code);
            for (size_t i = 0; i < code_len; ++i)
                ir_gen_node(node.block_node.code[i]);

            curr_vars = old_vars;

            return;
        }

        case NOD_UNARY_SUB: {
            ir_gen_node(*node.unary_node.value);

            inst_size = get_type_size(node.unary_node.value->d_type);

            ir_node = (ir_node_t){
                .dst = var_gen(inst_size),
                .instruction = INST_NEGATE,
                .size = inst_size,
                .op_1 = value
            };
            pback_array(&ir_code, &ir_node);

            value = ir_node.dst;
            return;
        }

        case NOD_BITWISE_NOT: {
            ir_gen_node(*node.unary_node.value);

            inst_size = get_type_size(node.unary_node.value->d_type);

            ir_node = (ir_node_t){
                .dst = var_gen(inst_size),
                .instruction = INST_BITWISE_NOT,
                .size = inst_size,
                .op_1 = value
            };
            pback_array(&ir_code, &ir_node);

            value = ir_node.dst;
            return;
        }

        case NOD_LOGICAL_NOT: {
            ir_gen_node(*node.unary_node.value);

            inst_size = get_type_size(node.unary_node.value->d_type);

            ir_node = (ir_node_t){
                .instruction = INST_LOGICAL_NOT,
                .size = inst_size,
                .op_1 = value,
                .dst = var_gen(4)
            };
            pback_array(&ir_code, &ir_node);

            value = ir_node.dst;
            return;
        }

        case NOD_BINARY_ADD: {
            ir_gen_binary_op(INST_ADD, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }

        case NOD_BINARY_SUB: {
            ir_gen_binary_op(INST_SUB, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }

        case NOD_BINARY_MUL: {
            ir_gen_binary_op(INST_MUL, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }

        case NOD_BINARY_DIV: {
            if (is_unsigned(node.binary_node.value_a->d_type))
                ir_gen_binary_op(INST_UDIV, node.binary_node.value_a, node.binary_node.value_b);
            else
                ir_gen_binary_op(INST_DIV, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }

        case NOD_BINARY_MOD: {
            if (is_unsigned(node.binary_node.value_a->d_type))
                ir_gen_binary_op(INST_UMOD, node.binary_node.value_a, node.binary_node.value_b);
            else
                ir_gen_binary_op(INST_MOD, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }

        case NOD_BITWISE_AND: {
            ir_gen_binary_op(INST_BITWISE_AND, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }

        case NOD_BITWISE_OR: {
            ir_gen_binary_op(INST_BITWISE_OR, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }

        case NOD_BITWISE_XOR: {
            ir_gen_binary_op(INST_BITWISE_XOR, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }

        case NOD_SHIFT_LEFT: {
            ir_gen_binary_op(INST_SHIFT_LEFT, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }
        
        case NOD_SHIFT_RIGHT: {
            ir_gen_binary_op(INST_SHIFT_RIGHT, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }

        case NOD_LOGICAL_AND: {
            ir_operand_t dst = var_gen(4);
            size_t false_label = label_count++;
            size_t end_label = label_count++;
            
            ir_gen_node(*node.binary_node.value_a);
            ir_node = (ir_node_t){
                .instruction = INST_JUMP_IF_Z,
                .size = get_type_size(node.binary_node.value_a->d_type),
                .op_1 = value,
                .dst.label_id = false_label
            };
            pback_array(&ir_code, &ir_node);

            ir_gen_node(*node.binary_node.value_b);
            ir_node = (ir_node_t){
                .instruction = INST_JUMP_IF_Z,
                .size = get_type_size(node.binary_node.value_b->d_type),
                .op_1 = value,
                .dst.label_id = false_label
            };
            pback_array(&ir_code, &ir_node);

            ir_node = (ir_node_t){
                .instruction = INST_COPY,
                .dst = dst,
                .op_1.type = IMM_U32, 
                .op_1.immediate = 1,
                .size = 4
            };
            pback_array(&ir_code, &ir_node);

            ir_node = (ir_node_t){
                .instruction = INST_JUMP,
                .dst.label_id = end_label
            };
            pback_array(&ir_code, &ir_node);

            ir_node = (ir_node_t){
                .instruction = INST_LABEL,
                .dst.label_id = false_label
            };
            pback_array(&ir_code, &ir_node);

            ir_node = (ir_node_t){
                .instruction = INST_COPY,
                .dst = dst,
                .op_1.type = IMM_U32, .op_1.immediate = 0,
                .size = 4
            };
            pback_array(&ir_code, &ir_node);

            ir_node = (ir_node_t){
                .instruction = INST_LABEL,
                .dst.label_id = end_label
            };
            pback_array(&ir_code, &ir_node);

            value = dst;

            break;
        }

        case NOD_LOGICAL_OR: {
            ir_operand_t dst = var_gen(4);
            size_t true_label = label_count++;
            size_t end_label = label_count++;

            ir_gen_node(*node.binary_node.value_a);

            ir_node = (ir_node_t){
                .instruction = INST_JUMP_IF_NZ,
                .size = get_type_size(node.binary_node.value_a->d_type),
                .dst.label_id = true_label,
                .op_1 = value
            };
            pback_array(&ir_code, &ir_node);

            ir_gen_node(*node.binary_node.value_b);

            ir_node = (ir_node_t){
                .instruction = INST_JUMP_IF_NZ,
                .size = get_type_size(node.binary_node.value_a->d_type),
                .dst.label_id = true_label,
                .op_1 = value
            };
            pback_array(&ir_code, &ir_node);

            ir_node = (ir_node_t){
                .instruction = INST_COPY,
                .dst = dst,
                .op_1   .type = IMM_U32,
                .op_1   .immediate = 0,
                .size = 4
            };

            ir_node = (ir_node_t){
                .instruction = INST_LABEL,
                .dst.label_id = true_label
        
            };
            pback_array(&ir_code, &ir_node);

            ir_node = (ir_node_t){
                .instruction = INST_COPY,
                .dst = dst,
                .op_1   .type = IMM_U32,
                .op_1   .immediate = 1,
                .size = 4
            };
            pback_array(&ir_code, &ir_node);

            ir_node = (ir_node_t){
                .instruction = INST_LABEL,
                .dst.label_id = end_label
            };
            pback_array(&ir_code, &ir_node);

            value = dst;
            
            break;
        }

        case NOD_EQUAL_TO: {
            ir_gen_binary_comp(INST_SET_E, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }
        
        case NOD_NOT_EQUAL_TO: {
            ir_gen_binary_comp(INST_SET_NE, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }
        
        case NOD_GREATER_THAN: {
            ir_gen_binary_comp(INST_SET_G, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }
        
        case NOD_LESS_THAN: {
            ir_gen_binary_comp(INST_SET_L, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }
        
        case NOD_GREATER_THAN_OR_EQUAL: {
            ir_gen_binary_comp(INST_SET_GE, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }
        
        case NOD_LESS_THAN_OR_EQUAL: {
            ir_gen_binary_comp(INST_SET_LE, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }

        case NOD_ASSIGN: {
            if (node.assign_node.destination->type == NOD_VARIABLE_ACCESS) {
                ir_gen_node(*node.assign_node.destination);
                ir_operand_t dst = value;
                ir_gen_node(*node.assign_node.source);
                
                ir_node = (ir_node_t){
                    .instruction = INST_COPY,
                    .size = get_type_size(node.assign_node.destination->d_type),
                    .op_1 = value,
                    .dst = dst,
                };
                pback_array(&ir_code, &ir_node);
            }
            else {
                void ir_gen_lvalue(node_t node);

                ir_gen_lvalue(*node.assign_node.destination);

                ir_operand_t dst_ptr = value;

                ir_gen_node(*node.assign_node.source);

                if (value.type == REG_R11 || value.type == MEM_ADDRESS){
                    ir_node = (ir_node_t){
                        .instruction = INST_COPY,
                        .op_1 = value,
                        .dst = var_gen(get_type_size(node.assign_node.destination->d_type)),
                        .size = get_type_size(node.assign_node.destination->d_type)
                    };
                    pback_array(&ir_code, &ir_node);

                    value = ir_node.dst;
                }

                ir_node = (ir_node_t){
                    .instruction = INST_COPY,
                    .dst = (ir_operand_t){.type = REG_R11},
                    .op_1 = dst_ptr,
                    .size = 8
                };
                pback_array(&ir_code, &ir_node);

                ir_node = (ir_node_t){
                    .instruction = INST_COPY,
                    .op_1 = value,
                    .dst.type = MEM_ADDRESS,
                    .size = get_type_size(node.assign_node.destination->d_type)
                };
                pback_array(&ir_code, &ir_node);
            }

            value = ir_node.op_1;

            return;
        }

        case NOD_VARIABLE_ACCESS: {
            value = all_vars[node.var_access_node.var_info.scope_id][node.var_access_node.var_info.var_num];
            return;
        }

        case NOD_PRE_INCREMENT: {
            if (node.unary_node.value->type == NOD_DEREFERENCE){
                ir_gen_node(*node.unary_node.value->unary_node.value);

                ir_node = (ir_node_t){
                    .instruction = INST_COPY,
                    .op_1 = value,
                    .dst = (ir_operand_t){
                        .type = REG_R11
                    },
                    .size = 8
                };
                pback_array(&ir_code, &ir_node);

                value.type = MEM_ADDRESS;
            }
            else
                ir_gen_node(*node.unary_node.value);
            
            
            ir_node = (ir_node_t){
                .instruction = INST_INCREMENT,
                .size = get_type_size(node.unary_node.value->d_type),
                .dst = value
            };
            pback_array(&ir_code, &ir_node);
            return;
        }

        case NOD_POST_INCREMENT: {
            if (node.unary_node.value->type == NOD_DEREFERENCE){
                ir_gen_node(*node.unary_node.value->unary_node.value);

                ir_node = (ir_node_t){
                    .instruction = INST_COPY,
                    .op_1 = value,
                    .dst = (ir_operand_t){
                        .type = REG_R11
                    },
                    .size = 8
                };
                pback_array(&ir_code, &ir_node);

                value.type = MEM_ADDRESS;
            }
            else
                ir_gen_node(*node.unary_node.value);

            inst_size = get_type_size(node.unary_node.value->d_type);

            ir_operand_t dst = var_gen(inst_size);

            ir_node = (ir_node_t){
                .instruction = INST_COPY,
                .size = inst_size,
                .op_1 = value,
                .dst = dst
            };
            pback_array(&ir_code, &ir_node);

            ir_node = (ir_node_t){
                .instruction = INST_INCREMENT,
                .size = inst_size,
                .dst = value
            };
            pback_array(&ir_code, &ir_node);

            value = dst;
            return;
        }

        case NOD_IF_STATEMENT: {
            ir_gen_node(*node.conditional_node.condition);

            size_t false_label = label_count++;
            size_t end_label = label_count++;

            ir_node = (ir_node_t){
                .instruction = INST_JUMP_IF_Z,
                .size = get_type_size(node.conditional_node.condition->d_type),
                .dst.label_id = false_label,
                .op_1 = value
            };
            pback_array(&ir_code, &ir_node);

            ir_gen_node(*node.conditional_node.true_block);

            ir_node = (ir_node_t){
                .instruction = INST_JUMP,
                .dst.label_id = end_label,
            };
            pback_array(&ir_code, &ir_node);

            ir_node = (ir_node_t){
                .instruction = INST_LABEL,
                .dst.label_id = false_label
            };
            pback_array(&ir_code, &ir_node);

            if (node.conditional_node.false_block)
                ir_gen_node(*node.conditional_node.false_block);

            ir_node = (ir_node_t){
                .instruction = INST_LABEL,
                .dst.label_id = end_label
            };
            pback_array(&ir_code, &ir_node);

            return;
        }

        case NOD_TERNARY_EXPRESSION: {
            inst_size = get_type_size(node.conditional_node.condition->d_type);

            ir_operand_t dst = var_gen(inst_size);

            ir_gen_node(*node.conditional_node.condition);

            ir_operand_t condition_val = value;

            size_t false_label = label_count++;
            size_t end_label = label_count++;

            ir_node = (ir_node_t){
                .instruction = INST_JUMP_IF_Z,
                .size = inst_size,
                .dst.label_id = false_label,
                .op_1 = condition_val
            };
            pback_array(&ir_code, &ir_node);

            ir_gen_node(*node.conditional_node.true_block);

            ir_node = (ir_node_t){
                .instruction = INST_COPY,
                .size = inst_size,
                .dst = dst,
                .op_1 = value
            };
            pback_array(&ir_code, &ir_node);

            ir_node = (ir_node_t){
                .instruction = INST_JUMP,
                .dst.label_id = end_label
            };
            pback_array(&ir_code, &ir_node);
            
            ir_node = (ir_node_t){
                .instruction = INST_LABEL,
                .dst.label_id = false_label
            };
            pback_array(&ir_code, &ir_node);

            ir_gen_node(*node.conditional_node.false_block);
            
            ir_node = (ir_node_t){
                .instruction = INST_COPY,
                .size = inst_size,
                .dst = dst,
                .op_1 = value
            };
            pback_array(&ir_code, &ir_node);

            ir_node = (ir_node_t){
                .instruction = INST_LABEL,
                .dst.label_id = end_label
            };
            pback_array(&ir_code, &ir_node);

            value = dst;

            return;
        }

        case NOD_WHILE_LOOP: {
            size_t old_continue_label = continue_label;
            size_t old_break_label = break_label;
            continue_label = label_count++;
            break_label = label_count++;

            ir_node = (ir_node_t){
                .instruction = INST_LABEL,
                .dst.label_id = continue_label
            };
            pback_array(&ir_code, &ir_node);

            ir_gen_node(*node.while_node.condition);
            
            ir_node = (ir_node_t){
                .instruction = INST_JUMP_IF_Z,
                .size = INST_TYPE_LONGWORD,
                .dst.label_id = break_label,
                .op_1 = value
            };
            pback_array(&ir_code, &ir_node);

            ir_gen_node(*node.while_node.code);

            ir_node = (ir_node_t){
                .instruction = INST_JUMP,
                .dst.label_id = continue_label
            };
            pback_array(&ir_code, &ir_node);

            ir_node = (ir_node_t){
                .instruction = INST_LABEL,
                .dst.label_id = break_label
            };
            pback_array(&ir_code, &ir_node);

            continue_label = old_continue_label;
            break_label = old_break_label;

            return;
        }

        case NOD_DO_WHILE_LOOP: {
            size_t old_continue_label = continue_label;
            size_t old_break_label = break_label;
            continue_label = label_count++;
            break_label = label_count++;
            
            ir_node = (ir_node_t){
                .instruction = INST_LABEL,
                .dst.label_id = continue_label
            };
            pback_array(&ir_code, &ir_node);

            ir_gen_node(*node.while_node.code);

            ir_gen_node(*node.while_node.condition);

            ir_node = (ir_node_t){
                .instruction = INST_JUMP_IF_Z,
                .size = INST_TYPE_LONGWORD,
                .dst.label_id = break_label,
                .op_1 = value
            };
            pback_array(&ir_code, &ir_node);

            ir_node = (ir_node_t){
                .instruction = INST_JUMP,
                .dst.label_id = continue_label,
            };
            pback_array(&ir_code, &ir_node);

            ir_node = (ir_node_t){
                .instruction = INST_LABEL,
                .dst.label_id = break_label,
            };
            pback_array(&ir_code, &ir_node);

            continue_label = old_continue_label;
            break_label = old_break_label;

            return;
        }

        case NOD_FOR_LOOP: {
            /* copy-pasted variable resolution code - since for-loops have their own scope */
            ir_operand_t* old_vars = curr_vars;
            {
                curr_vars = alloc_array(sizeof(ir_operand_t), 1);

                ir_operand_t variable;

                size_t var_amount = get_count_array(function_scope[node.for_node.scope_id]);
                for (size_t i = 0; i < var_amount; ++i){
                    if (function_scope[node.for_node.scope_id][i].data_type.storage_class == STORAGE_STATIC){
                        variable = (ir_operand_t){
                            .type = STATIC_MEM_LOCAL,
                            .identifier = function_scope[node.for_node.scope_id][i].name,
                        };

                        if (!function_scope[node.for_node.scope_id][i].init){
                            ir_node = (ir_node_t){
                                .instruction = INST_INIT_STATIC_Z_LOCAL,
                                .dst.identifier = function_scope[node.for_node.scope_id][i].name,
                                .op_2.label_id = get_type_size(function_scope[node.for_node.scope_id][i].data_type),
                                .size = func_num
                            };
                            pback_array(&zero_initializers, &ir_node);
                        }
                    } else
                    if (function_scope[node.for_node.scope_id][i].data_type.storage_class == STORAGE_EXTERN)
                        variable = (ir_operand_t){
                            .type = STATIC_MEM,
                            .identifier = function_scope[node.for_node.scope_id][i].name,
                        };
                    else{
                        frame_size += get_type_size(function_scope[node.for_node.scope_id][i].data_type);

                        variable = (ir_operand_t){
                            .type = STK_OFFSET,
                            .offset = -frame_size
                        };

                    }
                    pback_array(&curr_vars, &variable);
                }

                pback_array(&all_vars, &curr_vars);
            }

            size_t old_continue_label = continue_label;
            size_t old_break_label = break_label;   
            continue_label = label_count++;
            break_label = label_count++;
            size_t start_label = label_count++;

            ir_gen_node(*node.for_node.init);

            ir_node = (ir_node_t){
                .instruction = INST_LABEL,
                .dst.label_id = start_label
            };
            pback_array(&ir_code, &ir_node);

            ir_gen_node(*node.for_node.condition);

            ir_node = (ir_node_t){
                .instruction = INST_JUMP_IF_Z,
                .size = INST_TYPE_LONGWORD,
                .dst.label_id = break_label,
                .op_1 = value
            };
            pback_array(&ir_code, &ir_node);

            ir_gen_node(*node.for_node.code);

            ir_node = (ir_node_t){
                .instruction = INST_LABEL,
                .dst.label_id = continue_label
            };
            pback_array(&ir_code, &ir_node);

            ir_gen_node(*node.for_node.repeat);

            ir_node = (ir_node_t){
                .instruction = INST_JUMP,
                .dst.label_id = start_label
            };
            pback_array(&ir_code, &ir_node);

            ir_node = (ir_node_t){
                .instruction = INST_LABEL,
                .dst.label_id = break_label
            };
            pback_array(&ir_code, &ir_node);

            continue_label = old_continue_label;
            break_label = old_break_label;
            curr_vars = old_vars;

            return;
        }

        case NOD_LABEL: {
            ir_node = (ir_node_t){
                .instruction = INST_USER_LABEL,
                .dst.identifier = node.label_node.label
            };
            pback_array(&ir_code, &ir_node);

            return;
        }

        case NOD_GOTO: {
            ir_node = (ir_node_t){
                .instruction = INST_GOTO,
                .dst.identifier = node.label_node.label
            };
            pback_array(&ir_code, &ir_node);

            return;
        }

        case NOD_SWITCH_STATEMENT: {
            size_t old_break_label = break_label;
            break_label = label_count++;

            ir_gen_node(*node.switch_node.value);

            ir_operand_t compare_val = value;

            size_t old_old_label_count = old_label_count;

            old_label_count = label_count;

            label_count += get_count_array(node.switch_node.cases);

            size_t size = get_count_array(node.switch_node.cases);
            for (size_t i = 0; i < size; ++i){
                ir_gen_node(node.switch_node.cases[i]);
                ir_node = (ir_node_t){
                    .instruction =  INST_JUMP_IF_EQ,
                    .size = get_type_size(node.switch_node.value->d_type),
                    .dst.label_id = old_label_count + i,
                    .op_1 = compare_val,
                    .op_2 = value
                };
                pback_array(&ir_code, &ir_node);
            }

            size_t old_default_case_label = default_case_label;

            if (node.switch_node.default_case_defined){
                default_case_label = old_label_count + get_count_array(node.switch_node.cases);

                ir_node = (ir_node_t){
                    .instruction = INST_JUMP,
                    .dst.label_id = default_case_label
                };
                pback_array(&ir_code, &ir_node);
            }

            ir_node = (ir_node_t){
                .instruction =  INST_JUMP,
                .dst.label_id = break_label
            };
            pback_array(&ir_code, &ir_node);

            ir_gen_node((node_t){
                .type = NOD_BLOCK,
                .block_node = node.switch_node.code});

            default_case_label = old_default_case_label;


            ir_node = (ir_node_t){
                .instruction =  INST_LABEL,
                .dst.label_id = break_label
            };
            pback_array(&ir_code, &ir_node);

            break_label = old_break_label;

            ++label_count;

            return;
        }

        case NOD_SWITCH_CASE: {
            ir_node = (ir_node_t){
                .instruction =  INST_LABEL,
                .dst.label_id = node.case_node.case_id + old_label_count
            };
            pback_array(&ir_code, &ir_node);

            return;
        }

        case NOD_DEFAULT_CASE: {
            ir_node = (ir_node_t){
                .instruction = INST_LABEL,
                .dst.label_id = default_case_label
            };
            pback_array(&ir_code, &ir_node);
            return;
        }

        case NOD_FUNC_CALL: {
            ir_operand_t ir_gen_function_arg(size_t arg_num);

            ir_gen_node(*node.func_call_node.function);

            ir_operand_t func = value;
            
            ir_node = (ir_node_t){
                .instruction = INST_SAVE_CALLER_REGS
            };
            pback_array(&ir_code, &ir_node);

            size_t arg_num = get_count_array(node.func_call_node.args);
            size_t return_size = get_type_size(node.d_type);

            size_t generate_func_call_args(node_t* nod_args, size_t return_size);

            size_t offset = generate_func_call_args(node.func_call_node.args, return_size);

            ir_node = (ir_node_t){
                .instruction = INST_CALL,
                .dst = func
            };
            pback_array(&ir_code, &ir_node);

            /* deallocate space on the stack */
            ir_node = (ir_node_t){
                .instruction = INST_STACK_DEALLOC,
                .op_1.type = IMM_U32,
                .op_1.immediate = offset
            };
            pback_array(&ir_code, &ir_node);

            
            if (8 <= return_size && return_size <= 16){
                value = var_gen(return_size);

                ir_node = (ir_node_t){
                    .instruction = INST_COPY,
                    .op_1.type = REG_AX,
                    .dst = value,
                    .size = 8
                };
                pback_array(&ir_code, &ir_node);

                ir_node = (ir_node_t){
                    .instruction = INST_COPY,
                    .op_1.type = REG_DX,
                    .dst = get_val_offset(value, 8),
                    .size = return_size - 8
                };
                pback_array(&ir_code, &ir_node);
            }

            ir_node = (ir_node_t){
                .instruction = INST_LOAD_CALLER_REGS
            };
            pback_array(&ir_code, &ir_node);
            
            return;
        }

        case NOD_FUNC: {
            ir_gen_func(node.func_node);
            return;
        }

        case NOD_STATIC_INIT: {

            ir_gen_node(*node.static_init_node.value);

            ir_node = (ir_node_t){
                .dst.identifier = node.static_init_node.sym_info.name,
                .op_1 = value,
                .op_2.label_id = get_type_size(node.static_init_node.value->d_type),
                .size = func_num
            };

            if (node.static_init_node.global){
                if (node.static_init_node.sym_info.data_type.storage_class == STORAGE_STATIC)
                    ir_node.instruction = INST_INIT_STATIC;
                else
                    ir_node.instruction = INST_INIT_STATIC_PUBLIC;
            }
            else {
                if (node.static_init_node.sym_info.data_type.storage_class == STORAGE_STATIC)
                    ir_node.instruction = INST_INIT_STATIC_LOCAL;
            }

            pback_array(&static_initializers, &ir_node);
            return;
        }

        case NOD_TYPE_CAST: {
            ir_gen_node(*node.type_cast_node.src);
            /* handle all sign/zero -extend operations - if a sign/zero -extend is not necessary then the value is just the base value truncated */
            if ((node.type_cast_node.dst_type.base_type == TYPE_LONG || node.type_cast_node.dst_type.base_type == TYPE_UNSIGNED_LONG) && (node.type_cast_node.src->d_type.base_type == TYPE_INT || node.type_cast_node.src->d_type.base_type == TYPE_UNSIGNED_INT)){
                ir_node = (ir_node_t){
                    .dst = var_gen(8),
                    .op_1 = value,
                };

                if (is_unsigned(node.type_cast_node.src->d_type))
                    ir_node.instruction = INST_ZERO_EXTEND_LQ;
                else
                    ir_node.instruction = INST_SIGN_EXTEND_LQ;

                pback_array(&ir_code, &ir_node);

                value = ir_node.dst;
            } else
            if ((node.type_cast_node.dst_type.base_type == TYPE_SHORT || node.type_cast_node.dst_type.base_type == TYPE_UNSIGNED_SHORT) && (node.type_cast_node.src->d_type.base_type == TYPE_CHAR || node.type_cast_node.src->d_type.base_type == TYPE_UNSIGNED_CHAR)){
                ir_node = (ir_node_t){
                    .dst = var_gen(2),
                    .op_1 = value,
                };

                if (is_unsigned(node.type_cast_node.src->d_type))
                    ir_node.instruction = INST_ZERO_EXTEND_BW;
                else
                    ir_node.instruction = INST_SIGN_EXTEND_BW;

                pback_array(&ir_code, &ir_node);

                value = ir_node.dst;
            } else
            if ((node.type_cast_node.dst_type.base_type == TYPE_INT || node.type_cast_node.dst_type.base_type == TYPE_UNSIGNED_INT) && (node.type_cast_node.src->d_type.base_type == TYPE_CHAR || node.type_cast_node.src->d_type.base_type == TYPE_UNSIGNED_CHAR)){
                ir_node = (ir_node_t){
                    .dst = var_gen(4),
                    .op_1 = value,
                };

                if (is_unsigned(node.type_cast_node.src->d_type))
                    ir_node.instruction = INST_ZERO_EXTEND_BL;
                else
                    ir_node.instruction = INST_SIGN_EXTEND_BL;

                pback_array(&ir_code, &ir_node);

                value = ir_node.dst;
            } else
            if ((node.type_cast_node.dst_type.base_type == TYPE_LONG || node.type_cast_node.dst_type.base_type == TYPE_UNSIGNED_LONG) && (node.type_cast_node.src->d_type.base_type == TYPE_CHAR || node.type_cast_node.src->d_type.base_type == TYPE_UNSIGNED_CHAR)){
                ir_node = (ir_node_t){
                    .dst = var_gen(8),
                    .op_1 = value,
                };
                
                if (is_unsigned(node.type_cast_node.src->d_type))
                    ir_node.instruction = INST_ZERO_EXTEND_BQ;
                else
                    ir_node.instruction = INST_SIGN_EXTEND_BQ;

                pback_array(&ir_code, &ir_node);

                value = ir_node.dst;
            } else
            if ((node.type_cast_node.dst_type.base_type == TYPE_INT || node.type_cast_node.dst_type.base_type == TYPE_UNSIGNED_INT) && (node.type_cast_node.src->d_type.base_type == TYPE_SHORT || node.type_cast_node.src->d_type.base_type == TYPE_UNSIGNED_SHORT)){
                ir_node = (ir_node_t){
                    .dst = var_gen(4),
                    .op_1 = value,
                };

                if (is_unsigned(node.type_cast_node.src->d_type))
                    ir_node.instruction = INST_ZERO_EXTEND_WL;
                else
                    ir_node.instruction = INST_SIGN_EXTEND_WL;

                pback_array(&ir_code, &ir_node);

                value = ir_node.dst;
            } else
            if ((node.type_cast_node.dst_type.base_type == TYPE_LONG || node.type_cast_node.dst_type.base_type == TYPE_UNSIGNED_LONG) && (node.type_cast_node.src->d_type.base_type == TYPE_SHORT || node.type_cast_node.src->d_type.base_type == TYPE_UNSIGNED_SHORT)){
                ir_node = (ir_node_t){
                    .dst = var_gen(8),
                    .op_1 = value,
                };

                if (is_unsigned(node.type_cast_node.src->d_type))
                    ir_node.instruction = INST_ZERO_EXTEND_WQ;
                else
                    ir_node.instruction = INST_SIGN_EXTEND_WQ;

                pback_array(&ir_code, &ir_node);

                value = ir_node.dst;
            }
            
            return;
        }

        case NOD_BREAK: {
            ir_node = (ir_node_t){
                .instruction = INST_JUMP,
                .dst.label_id = break_label
            };
            pback_array(&ir_code, &ir_node);

            return;
        }

        case NOD_CONTINUE: {
            ir_node = (ir_node_t){
                .instruction = INST_JUMP,
                .dst.label_id = continue_label
            };
            pback_array(&ir_code, &ir_node);

            return;
        }

        case NOD_DEREFERENCE: {
            ir_gen_node(*node.unary_node.value);
            ir_node = (ir_node_t){
                .instruction = INST_LOAD_ADDRESS,
                .op_1 = value,
                .dst = var_gen(get_type_size(*node.unary_node.value->d_type.ptr_derived_type)),
                .size = get_type_size(*node.unary_node.value->d_type.ptr_derived_type)
            };
            pback_array(&ir_code, &ir_node);

            value = ir_node.dst;

            return;
        }

        case NOD_REFERENCE: {
            ir_gen_node(*node.unary_node.value);
            ir_node = (ir_node_t){
                .instruction = INST_GET_ADDRESS,
                .op_1 = value,
                .dst = (ir_operand_t){.type = REG_R11},
                .size = INST_TYPE_QUADWORD
            };
            pback_array(&ir_code, &ir_node);

            value = ir_node.dst;

            return;
        }

        case NOD_ADD_POINTER: {
            ir_gen_node(*node.binary_node.value_a);
            ir_operand_t ptr = value;
            ir_gen_node(*node.binary_node.value_b);

            ir_node = (ir_node_t){
                .instruction = INST_ADD_POINTER,
                .dst = var_gen(8),
                .op_1 = ptr,
                .op_2 = value,
                .size = get_type_size(*node.binary_node.value_a->d_type.ptr_derived_type)
            };
            pback_array(&ir_code, &ir_node);

            value = ir_node.dst;

            return;
        }

        case NOD_SUB_POINTER: {
            ir_gen_node(*node.binary_node.value_a);
            ir_operand_t ptr = value;
            ir_gen_node(*node.binary_node.value_b);
            
            ir_node = (ir_node_t){
                .instruction = INST_SUB_POINTER,
                .dst = var_gen(8),
                .op_1 = ptr,
                .op_2 = value,
                .size = get_type_size(*node.binary_node.value_a->d_type.ptr_derived_type)
            };
            pback_array(&ir_code, &ir_node);

            value = ir_node.dst;

            return;
        }

        case NOD_INIT_ARRAY: {
            if (node.array_init_node.elems->type == NOD_STRING_LITERAL){
                node_t* str_array = alloc_array(sizeof(node_t), 1);

                for (char* c = node.array_init_node.elems->const_node.str_literal; *c; c++){
                    node_t _node = (node_t){
                        .type = NOD_CHAR,
                        .const_node.char_literal = *c,
                        .d_type = (data_type_t){
                            .base_type = TYPE_CHAR,
                            .is_signed = true,
                        }
                    };

                    pback_array(&str_array, &_node);
                }

                node_t _node = (node_t){
                    .type = NOD_CHAR,
                    .const_node.char_literal = 0,
                    .d_type = (data_type_t){
                        .base_type = TYPE_CHAR,
                        .is_signed = true,
                    }
                };

                pback_array(&str_array, &_node);

                parse_node_dstr(node.array_init_node.elems);
                free(node.array_init_node.elems);

                node_t* make_node(node_t node);

                data_type_t* base_type = malloc(sizeof(data_type_t));
                *base_type = (data_type_t){
                    .base_type = TYPE_CHAR,
                    .is_signed = true,
                };

                data_type_t* other_base_type = malloc(sizeof(data_type_t));
                *other_base_type = *base_type;

                node.array_init_node.elems = make_node((node_t){
                    .type = NOD_ARRAY_LITERAL,
                    .array_literal_node.elems = str_array,
                    .array_literal_node.type = (data_type_t){
                        .base_type = TYPE_ARRAY,
                        .array_size = get_count_array(str_array),
                        .ptr_derived_type = base_type
                    },
                    .d_type = (data_type_t){
                        .base_type = TYPE_ARRAY,
                        .array_size = get_count_array(str_array),
                        .ptr_derived_type = other_base_type
                    },
                });

            }

            ir_gen_init_compound_literal(node.array_init_node.elems, all_vars[node.array_init_node.array_var.scope_id][node.array_init_node.array_var.var_num]);

            return;
        }

        case NOD_ARRAY_LITERAL: {
            ir_operand_t array_start = var_gen(get_type_size(node.d_type));

            data_type_t base_type = node.array_literal_node.type;

            while (base_type.base_type == TYPE_ARRAY)
                base_type = *base_type.ptr_derived_type;

            size_t count = 0;

            ir_gen_init_compound_literal(&node, array_start);

            value = array_start;

            return;
        }

        case NOD_STATIC_ARRAY_INIT: {
            if (node.static_init_node.value->type == NOD_STRING_LITERAL){
                if (node.static_init_node.sym_info.data_type.base_type == TYPE_POINTER){
                    ir_gen_node(*node.static_init_node.value);

                    ir_node = (ir_node_t){
                        .dst.identifier = node.static_init_node.sym_info.name,
                        .op_1.label_id = value.label_id
                    };

                    if (node.static_init_node.global){
                        if (node.static_init_node.sym_info.data_type.storage_class == STORAGE_STATIC)
                            ir_node.instruction = INST_STATIC_STRING_P;
                        else
                            ir_node.instruction = INST_STATIC_STRING_P_PUBLIC;
                    }
                    else
                        ir_node.instruction = INST_STATIC_STRING_P_LOCAL;

                    pback_array(&static_initializers, &ir_node);

                    return;
                }
                else {
                    node_t* str_array = alloc_array(sizeof(node_t), 1);

                    for (char* c = node.static_init_node.value->const_node.str_literal; *c; c++){
                        node_t _node = (node_t){
                            .type = NOD_CHAR,
                            .const_node.char_literal = *c,
                            .d_type = (data_type_t){
                                .base_type = TYPE_CHAR,
                                .is_signed = true,
                            }
                        };

                        pback_array(&str_array, &_node);
                    }

                    node_t _node = (node_t){
                        .type = NOD_CHAR,
                        .const_node.char_literal = 0,
                        .d_type = (data_type_t){
                            .base_type = TYPE_CHAR,
                            .is_signed = true,
                        }
                    };

                    pback_array(&str_array, &_node);

                    parse_node_dstr(node.static_init_node.value);
                    free(node.static_init_node.value);

                    node_t* make_node(node_t node);

                    data_type_t* base_type = malloc(sizeof(data_type_t));
                    *base_type = (data_type_t){
                        .base_type = TYPE_CHAR,
                        .is_signed = true,
                    };

                    data_type_t* other_base_type = malloc(sizeof(data_type_t));
                    *other_base_type = *base_type;

                    node.static_init_node.value = make_node((node_t){
                        .type = NOD_ARRAY_LITERAL,
                        .array_literal_node.elems = str_array,
                        .array_literal_node.type = (data_type_t){
                            .base_type = TYPE_ARRAY,
                            .array_size = get_count_array(str_array),
                            .ptr_derived_type = base_type
                        },
                        .d_type = (data_type_t){
                            .base_type = TYPE_ARRAY,
                            .array_size = get_count_array(str_array),
                            .ptr_derived_type = other_base_type
                        },
                    });
                }
            }

            data_type_t base_type = node.static_init_node.value->array_literal_node.type;

            while (base_type.base_type == TYPE_ARRAY)
                base_type = *base_type.ptr_derived_type;

            ir_node = (ir_node_t){
                .dst.identifier = node.static_init_node.sym_info.name,
                .op_1.label_id = get_type_size(node.static_init_node.value->array_literal_node.type),
                .op_2.label_id = get_type_size(base_type),
                .size = func_num
            };

            if (node.static_init_node.global)
                if (node.static_init_node.sym_info.data_type.storage_class == STORAGE_STATIC)
                    ir_node.instruction = INST_STATIC_ARRAY;
                else
                    ir_node.instruction = INST_STATIC_ARRAY_PUBLIC;
            else
                if (node.static_init_node.sym_info.data_type.storage_class == STORAGE_STATIC)
                    ir_node.instruction = INST_STATIC_ARRAY_LOCAL;

            pback_array(&static_initializers, &ir_node);

            ir_gen_static_compound_literal(node.static_init_node.value);

            return;
        }

        case NOD_SUB_POINTER_POINTER: {
            ir_gen_binary_op(INST_SUB, node.binary_node.value_a, node.binary_node.value_b);

            ir_node = (ir_node_t){
                .instruction = INST_UDIV,
                .op_1 = value,
                .op_2 = (ir_operand_t){
                    .type = IMM_U32,
                    .immediate = get_type_size(*node.binary_node.value_a->d_type.ptr_derived_type)
                },
                .dst = var_gen(INST_TYPE_QUADWORD),
                .size = INST_TYPE_QUADWORD
            };
            pback_array(&ir_code, &ir_node);

            value = ir_node.dst;

            return;
        }

        case NOD_STRING_LITERAL: {
            static size_t string_id = 0;

            ir_node = (ir_node_t){
                .instruction = INST_STATIC_STRING,
                .dst.identifier = node.const_node.str_literal,
                .op_1.label_id = string_id
            };
            pback_array(&read_only_initializers, &ir_node);

            value = (ir_operand_t){
                .type = STRING,
                .label_id = string_id++
            };

            return;
        }

        case NOD_SIZEOF_TYPE: {
            value = (ir_operand_t){
                .type = IMM_U64,
                .immediate = get_type_size(node.sizeof_type_node.d_type)
            };

            return;
        }

        case NOD_SIZEOF_EXPR: {
            value = (ir_operand_t){
                .type = IMM_U64,
                .immediate = get_type_size(node.unary_node.value->d_type)
            };

            return;
        }

        case NOD_STRUCTURE_MEMBER_ACCESS: {
            ir_gen_node(*node.member_access_node._struct);

            size_t get_structure_member_size(char* _struct, char* member);
            size_t get_structure_member_offset(char* _struct, char* member);

            value = get_val_offset(value, get_structure_member_offset(node.member_access_node._struct->d_type.struct_tag, node.member_access_node.member));

            return;
        }

        case NOD_STRUCTURE_POINTER_MEMBER_ACCESS: {
            ir_gen_node(*node.member_access_node._struct);

            size_t get_structure_member_size(char* _struct, char* member);
            size_t get_structure_member_offset(char* _struct, char* member);

            ir_node = (ir_node_t){
                .instruction = INST_ADD,
                .op_1 = value,
                .op_2 = (ir_operand_t){.type = IMM_U64, .immediate = get_structure_member_offset(node.member_access_node._struct->d_type.ptr_derived_type->struct_tag, node.member_access_node.member)},
                .dst.type = REG_R11,
                .size = 8
            };
            pback_array(&ir_code, &ir_node);

            value.type = MEM_ADDRESS;

            return;
        }

        case NOD_SUBSCRIPT: {
            ir_gen_node(*node.binary_node.value_a);
            ir_operand_t ptr = value;
            ir_gen_node(*node.binary_node.value_b);

            size_t type_size = get_type_size(*node.binary_node.value_a->d_type.ptr_derived_type);

            ir_node = (ir_node_t){
                .instruction = INST_ADD_POINTER,
                .dst = (ir_operand_t){.type = REG_R11 },
                .op_1 = ptr,
                .op_2 = value,
                .size = type_size
            };
            pback_array(&ir_code, &ir_node);

            ir_node = (ir_node_t){
                .instruction = INST_COPY,
                .dst = var_gen(type_size),
                .op_1 = (ir_operand_t){.type = MEM_ADDRESS },
                .size = type_size
            };
            pback_array(&ir_code, &ir_node);

            value = ir_node.dst;

            return;
        }

        case NOD_STRUCTURE_LITERAL: {
            ir_operand_t struct_start = var_gen(get_type_size(node.d_type));
            
            ir_gen_init_compound_literal(&node, struct_start);

            value = struct_start;
            return;
        }

        case NOD_INIT_STRUCTURE: {
            ir_gen_node(*node.assign_node.destination);
            ir_operand_t struct_start = value;
            ir_gen_init_compound_literal(node.assign_node.source, value);
            value = struct_start;
            return;
        }

        case NOD_STATIC_STRUCT_INIT: {
            ir_node = (ir_node_t){
                .dst.identifier = node.static_init_node.sym_info.name,
                .op_1.label_id = get_type_size(node.static_init_node.value->d_type),
                .size = func_num
            };

            if (node.static_init_node.global)
                if (node.static_init_node.sym_info.data_type.storage_class == STORAGE_STATIC)
                    ir_node.instruction = INST_STATIC_ARRAY;
                else
                    ir_node.instruction = INST_STATIC_ARRAY_PUBLIC;
            else
                if (node.static_init_node.sym_info.data_type.storage_class == STORAGE_STATIC)
                    ir_node.instruction = INST_STATIC_ARRAY_LOCAL;

            pback_array(&static_initializers, &ir_node);

            ir_gen_static_compound_literal(node.static_init_node.value);

            return;
        }

        default: {
            perror("unsupported expression");
            exit(-1);
        }
    }
}

size_t get_structure_member_offset(char *_struct, char *member);

void ir_gen_init_compound_literal(node_t* literal, ir_operand_t start){
    if (literal->type == NOD_ARRAY_LITERAL){
        for (size_t i = 0; i < get_count_array(literal->array_literal_node.elems); ++i){
            if (literal->array_literal_node.elems[i].type == NOD_ARRAY_LITERAL || literal->array_literal_node.elems[i].type == NOD_STRUCTURE_LITERAL)
                ir_gen_init_compound_literal(literal->array_literal_node.elems + i, get_val_offset(start, i * get_type_size(*literal->array_literal_node.type.ptr_derived_type)));
            else
            if (literal->array_literal_node.elems[i].type == NOD_STRING_LITERAL){
                node_t* convert_string_literal_to_array_literal(node_t* node);

                node_t char_elems = (node_t){
                    .type = NOD_ARRAY_LITERAL,
                    .array_literal_node.elems = convert_string_literal_to_array_literal(literal->array_literal_node.elems + i),
                    .array_literal_node.type = (data_type_t){
                        .base_type = TYPE_ARRAY,
                        .array_size = strlen(literal->array_literal_node.elems[i].const_node.str_literal) + 1,
                        .ptr_derived_type = malloc(sizeof(data_type_t))
                    },
                    .d_type = (data_type_t){
                        .base_type = TYPE_ARRAY,
                        .array_size = strlen(literal->array_literal_node.elems[i].const_node.str_literal) + 1,
                        .ptr_derived_type = malloc(sizeof(data_type_t))
                    },
                };
                *char_elems.array_literal_node.type.ptr_derived_type = (data_type_t){
                    .base_type = TYPE_CHAR,
                    .is_signed = true
                };
                *char_elems.d_type.ptr_derived_type = *char_elems.array_literal_node.type.ptr_derived_type;

                ir_gen_init_compound_literal(&char_elems, get_val_offset(start, i * get_type_size(*literal->array_literal_node.type.ptr_derived_type)));
            } else
            if (literal->array_literal_node.elems[i].type == NOD_REFERENCE && literal->array_literal_node.elems[i].unary_node.value->type == NOD_STRING_LITERAL){
                ir_gen_node(*literal->array_literal_node.elems[i].unary_node.value);
                value.type = STRING_ADDRESS;
                goto add_node;
            }
            else {
                ir_gen_node(literal->array_literal_node.elems[i]);
                
                ir_node_t ir_node;

                add_node:

                ir_node = (ir_node_t){
                    .instruction = INST_COPY,
                    .op_1 = value,
                    .dst = get_val_offset(start, i * get_type_size(literal->array_literal_node.elems[i].d_type)),
                    .size = get_type_size(literal->array_literal_node.elems[i].d_type)
                };
                pback_array(&ir_code, &ir_node);
            }
        }
    } else
    if (literal->type == NOD_STRUCTURE_LITERAL){
        for (size_t i = 0; i < get_count_array(literal->struct_literal_node.members); ++i){
            if (literal->struct_literal_node.values[i].type == NOD_ARRAY_LITERAL || literal->struct_literal_node.values[i].type == NOD_STRUCTURE_LITERAL){
                ir_gen_init_compound_literal(literal->struct_literal_node.values + i, get_val_offset(start, get_structure_member_offset(literal->d_type.struct_tag, literal->struct_literal_node.members[i])));
            } 
            else {
                ir_gen_node(literal->struct_literal_node.values[i]);

                ir_node_t ir_node = (ir_node_t){
                    .instruction = INST_COPY,
                    .op_1 = value,
                    .dst = get_val_offset(start, get_structure_member_offset(literal->d_type.struct_tag, literal->struct_literal_node.members[i])),
                    .size = get_type_size(literal->struct_literal_node.values[i].d_type)
                };
                pback_array(&ir_code, &ir_node);
            }
        }
    }
}

symbol_t *find_symbol(scope_t *scope, symbol_t target, size_t *scope_id_out, size_t *var_num_out);
node_t* convert_string_literal_to_array_literal(node_t* node);
size_t get_type_alignment(data_type_t type);

void ir_gen_static_compound_literal(node_t* literal){
    if (literal->type == NOD_ARRAY_LITERAL){
        size_t base_type_size = get_type_size(*literal->array_literal_node.type.ptr_derived_type);
        for (size_t i = 0; i < get_count_array(literal->array_literal_node.elems); ++i){
            if (literal->array_literal_node.elems[i].type == NOD_ARRAY_LITERAL || literal->array_literal_node.elems[i].type == NOD_STRUCTURE_LITERAL)
                ir_gen_static_compound_literal(literal->array_literal_node.elems + i); else
            if (literal->array_literal_node.elems[i].type == NOD_REFERENCE && literal->array_literal_node.elems[i].unary_node.value->type == NOD_STRING_LITERAL){
                ir_gen_node(*literal->array_literal_node.elems[i].unary_node.value);
                value.type = STRING_ADDRESS;
                goto add_node;
            }
            else {
                ir_gen_node(literal->array_literal_node.elems[i]);

                ir_node_t ir_node;
                
                add_node:

                ir_node = (ir_node_t){
                    .instruction = INST_STATIC_ELEM,
                    .op_1 = value,
                    .op_2.label_id = base_type_size,
                    .size = base_type_size
                };
                pback_array(&static_initializers, &ir_node);
            }
        }
    } else
    if (literal->type == NOD_STRUCTURE_LITERAL){
        symbol_t* _struct = find_symbol(global_scope, (symbol_t){.name = literal->d_type.struct_tag, .symbol_type = SYM_STRUCTURE}, NULL, NULL);
        
        size_t size = 0;
        size_t largest_elem_alignment = 0;

        for (size_t i = 0; i < get_count_array(_struct->struct_members); ++i){
            for (size_t j = 0; j < get_count_array(literal->struct_literal_node.members); ++j){
                if (!strcmp(_struct->struct_members[i].name, literal->struct_literal_node.members[j])){
                    size_t elem_size = get_type_size(_struct->struct_members[i].data_type);
                    size_t elem_alignment = get_type_alignment(_struct->struct_members[i].data_type);

                    /* padding required */
                    if (size % elem_alignment){
                        size_t padding = elem_size - size % elem_alignment;
                        ir_node_t ir_node = (ir_node_t){
                            .instruction = INST_ZERO,
                            .op_1.label_id = padding
                        };
                        pback_array(&static_initializers, &ir_node);
                        size += padding;
                    }

                    size += elem_size;
                    
                    if (largest_elem_alignment < elem_alignment)
                        largest_elem_alignment = elem_alignment;
                    
                    if (literal->struct_literal_node.values[j].type == NOD_ARRAY_LITERAL || literal->struct_literal_node.values[j].type == NOD_STRUCTURE_LITERAL)
                        ir_gen_static_compound_literal(literal->struct_literal_node.values + j); else
                    if (literal->struct_literal_node.values[j].type == NOD_REFERENCE && literal->struct_literal_node.values[j].unary_node.value->type == NOD_STRING_LITERAL){
                        ir_gen_node(*literal->struct_literal_node.values[j].unary_node.value);
                        value.type = STRING_ADDRESS;
                        goto add_node_1;
                    }
                    else {
                        ir_gen_node(literal->struct_literal_node.values[j]);

                        ir_node_t ir_node;

                        add_node_1:

                        ir_node = (ir_node_t){
                            .instruction = INST_STATIC_ELEM,
                            .op_1 = value,
                            .op_2.label_id = get_type_size(_struct->struct_members[i].data_type),
                            .size = get_type_size(_struct->struct_members[i].data_type)
                        };
                        pback_array(&static_initializers, &ir_node);
                    }

                    goto member_asssigned;
                }
            }
            /* else */ {
                ir_node_t ir_node = (ir_node_t){
                    .instruction = INST_ZERO,
                    .op_1.label_id = get_type_size(_struct->struct_members[i].data_type),
                };
                pback_array(&static_initializers, &ir_node);
            }
            member_asssigned:;
        }
    
        if (size % largest_elem_alignment){
            ir_node_t ir_node = (ir_node_t){
                .instruction = INST_ZERO,
                .op_1.label_id = largest_elem_alignment - size % largest_elem_alignment
            };
            pback_array(&static_initializers, &ir_node);
        }
    }
}

static size_t get_type_size(data_type_t type){
    switch(type.base_type){
        case TYPE_INT:
        case TYPE_UNSIGNED_INT:
            return 4;
        case TYPE_LONG:
        case TYPE_UNSIGNED_LONG:
        case TYPE_POINTER:
        case TYPE_FUNCTION_POINTER:
            return 8;
        case TYPE_SHORT:
        case TYPE_UNSIGNED_SHORT:
            return 2;
        case TYPE_CHAR:
        case TYPE_UNSIGNED_CHAR:
            return 1;
        case TYPE_ARRAY:
            return get_type_size(*type.ptr_derived_type) * type.array_size;
        case TYPE_VOID:
            return 1; /* im lazy, sue me */
        case TYPE_STRUCT: {

            symbol_t *find_symbol(scope_t *scope, symbol_t target, size_t *scope_id_out, size_t *var_num_out);
            size_t get_type_alignment(data_type_t type);

            symbol_t* _struct = find_symbol(global_scope, (symbol_t){.name = type.struct_tag, .symbol_type = SYM_STRUCTURE}, NULL, NULL);

            size_t size = get_type_size(_struct->struct_members[0].data_type);
            size_t largest_elem_alignment = size;

            for (size_t i = 1; i < get_count_array(_struct->struct_members); ++i){
                size_t elem_size = get_type_size(_struct->struct_members[i].data_type);
                size_t elem_alignment = get_type_alignment(_struct->struct_members[i].data_type);

                if (size % elem_alignment)
                    size += elem_size - (size % elem_size);

                if (largest_elem_alignment < elem_alignment)
                    largest_elem_alignment = elem_alignment;

                size += elem_size;
            }

            if (size % largest_elem_alignment)
                size += largest_elem_alignment - (size % largest_elem_alignment);

            return size;
        }
        default:
            return 0;
    }
}

void ir_node_dstr(void* _node){
    ir_node_t* node = _node;
    switch(node->instruction){
        default:
            return;
    }
}

ir_operand_t ir_gen_function_arg(size_t arg_num){
    
    ir_operand_t result;
    
    switch(arg_num){
        case 1:
            result.type = REG_DI;
            break;
        case 2:
            result.type = REG_SI;
            break;
        case 3:
            result.type = REG_DX;
            break;
        case 4:
            result.type = REG_CX;
            break;
        case 5:
            result.type = REG_R8;
            break;
        case 6:
            result.type = REG_R9;
            break;
        default:
            result.type = STK_OFFSET;
            result.offset = ((arg_num - 6) * 8) + 16;
            break;
    }

    return result;
}

node_t* convert_string_literal_to_array_literal(node_t* node){
    node_t* result = alloc_array(sizeof(node_t), 1);

    node_t elem;
    
    for (char* c = node->const_node.str_literal; *c; ++c){
        elem = (node_t){
            .type = NOD_CHAR,
            .const_node.char_literal = *c,
            .d_type = (data_type_t){
                .base_type = TYPE_CHAR,
                .is_signed = true,
                .storage_class = STORAGE_NULL
            }
        };
        pback_array(&result, &elem);
    }


    elem = (node_t){
        .type = NOD_CHAR,
        .const_node.char_literal = 0,
        .d_type = (data_type_t){
            .base_type = TYPE_CHAR,
            .is_signed = true,
            .storage_class = STORAGE_NULL
        }
    };
    pback_array(&result, &elem);

    return result;
}

size_t get_structure_member_size(char* _struct, char* member){
    size_t size = 0;

    

    symbol_t* __struct = find_symbol(global_scope, (symbol_t){.name = _struct, .symbol_type = SYM_STRUCTURE}, NULL, NULL);

    for (size_t i = 0; i < get_count_array(__struct->struct_members); ++i){
        if (!strcmp(__struct->struct_members[i].name, member))
            return get_type_size(__struct->struct_members[i].data_type);
    }

    return size;
}

size_t get_structure_member_offset(char* _struct, char* member){

    symbol_t *find_symbol(scope_t *scope, symbol_t target, size_t *scope_id_out, size_t *var_num_out);
    size_t get_type_alignment(data_type_t type);

    symbol_t* __struct = find_symbol(global_scope, (symbol_t){.name = _struct, .symbol_type = SYM_STRUCTURE}, NULL, NULL);

    size_t offset = get_type_size(__struct->struct_members[0].data_type);

    for (size_t i = 1; i < get_count_array(__struct->struct_members); ++i){
        
        size_t elem_size = get_type_size(__struct->struct_members[i].data_type);
        size_t elem_alignment = get_type_alignment(__struct->struct_members[i].data_type);

        if (offset % elem_alignment)
            offset += elem_alignment - (offset % elem_alignment);

        if (!strcmp(__struct->struct_members[i].name, member))
            return offset;
        
        offset += elem_size;
    }

    return 0;
}

/* sets value = (pointer to result) */
void ir_gen_lvalue(node_t node){
    ir_node_t ir_node;
    switch(node.type){
        case NOD_DEREFERENCE: {
            ir_gen_node(*node.unary_node.value);

            return;
        }

        case NOD_STRUCTURE_MEMBER_ACCESS: {
            size_t get_structure_member_offset(char* _struct, char* member);
            
            /* will always give an lvalue-ish */
            ir_gen_node(*node.member_access_node._struct);

            value = get_val_offset(value, get_structure_member_offset(node.member_access_node._struct->d_type.struct_tag, node.member_access_node.member));

            ir_node = (ir_node_t){
                .instruction = INST_GET_ADDRESS,
                .op_1 = value,
                .dst = var_gen(8),
                .size = 8
            };
            pback_array(&ir_code, &ir_node);

            value = ir_node.dst;

            return;
        }

        case NOD_STRUCTURE_POINTER_MEMBER_ACCESS: {
            ir_gen_node(*node.member_access_node._struct);

            ir_node = (ir_node_t){
                .instruction = INST_ADD,
                .op_1 = value,
                .op_2 = (ir_operand_t){
                    .type = IMM_U64,
                    .immediate = get_structure_member_offset(node.member_access_node._struct->d_type.ptr_derived_type->struct_tag, node.member_access_node.member)
                },
                .dst = var_gen(8),
                .size = 8
            };
            pback_array(&ir_code, &ir_node);

            value = ir_node.dst;

            return;
        }

        case NOD_SUBSCRIPT: {
            /* dont need to gen an lvalue since its a pointer */
            ir_gen_lvalue(*node.binary_node.value_a);

            ir_operand_t ptr_ptr = value;
            
            ir_gen_node(*node.binary_node.value_b);

            if (value.type == MEM_ADDRESS){
                ir_node = (ir_node_t){
                    .instruction = INST_COPY,
                    .op_1 = value,
                    .dst = var_gen(get_type_size(node.binary_node.value_b->d_type)),
                    .size = 8
                };
                pback_array(&ir_code, &ir_node);

                value = ir_node.dst;
            }

            ir_node = (ir_node_t){
                .instruction = INST_COPY,
                .op_1 = ptr_ptr,
                .dst = (ir_operand_t){.type = REG_R11},
                .size = 8
            };
            pback_array(&ir_code, &ir_node);

            ir_node = (ir_node_t){
                .instruction = INST_ADD_POINTER,
                .dst = var_gen(8),
                .op_1.type = MEM_ADDRESS,
                .op_2 = value,
                .size = get_type_size(*node.binary_node.value_a->d_type.ptr_derived_type)
            };
            pback_array(&ir_code, &ir_node);

            value = ir_node.dst;

            return;
        }

        case NOD_VARIABLE_ACCESS: {
            ir_gen_node(node);

            ir_node = (ir_node_t){
                .instruction = INST_GET_ADDRESS,
                .op_1 = value,
                .dst = var_gen(8),
                .size = 8
            };
            pback_array(&ir_code, &ir_node);

            value = ir_node.dst;

            return;
        }

        case NOD_REFERENCE: {
            if (node.unary_node.value->d_type.base_type == TYPE_ARRAY){
                ir_gen_lvalue(*node.unary_node.value);

                ir_node = (ir_node_t){
                    .instruction = INST_GET_ADDRESS,
                    .op_1 = value,
                    .dst = var_gen(8),
                    .size = INST_TYPE_QUADWORD
                };
                pback_array(&ir_code, &ir_node);

                value = ir_node.dst;

                return;
            }
        }

        default: {
            fprintf(stderr, "Error: Invalid lvalue\n");
            exit(-1);
        }
    }
}

size_t get_largest_structure_member(char* _struct){
    symbol_t *find_symbol(scope_t *scope, symbol_t target, size_t *scope_id_out, size_t *var_num_out);
    
    symbol_t* __struct = find_symbol(global_scope, (symbol_t){.name = _struct, .symbol_type = SYM_STRUCTURE}, NULL, NULL);

    size_t largest_size  = get_type_size(__struct->struct_members[0].data_type);

    for (size_t i = 1; i < get_count_array(__struct->struct_members); ++i){
        size_t elem_size;
        if (__struct->struct_members[i].data_type.base_type == TYPE_STRUCT){
            elem_size = get_largest_structure_member(__struct->struct_members[i].data_type.struct_tag);
            if (largest_size < elem_size)
                largest_size = elem_size;
        }
        else {
            elem_size = get_type_size(__struct->struct_members[i].data_type);
            if (largest_size < elem_size)
                largest_size = elem_size;
        }
    }

    return largest_size;
}

size_t get_type_alignment(data_type_t type){
    switch(type.base_type){
        case TYPE_ARRAY: {
            while (type.base_type == TYPE_ARRAY)
                type = *type.ptr_derived_type;

            return get_type_alignment(type);
        }

        case TYPE_STRUCT: {
            symbol_t *find_symbol(scope_t *scope, symbol_t target, size_t *scope_id_out, size_t *var_num_out);
            symbol_t* _struct = find_symbol(global_scope, (symbol_t){.name = type.struct_tag, .symbol_type = SYM_STRUCTURE}, NULL, NULL);

            size_t largest_alignment = 0;
            size_t elem_alignment;

            for (size_t i = 0; i < get_count_array(_struct->struct_members); ++i){
                elem_alignment = get_type_alignment(_struct->struct_members[i].data_type);
                if (largest_alignment < elem_alignment)
                    largest_alignment = elem_alignment;
            }

            return largest_alignment;
        }

        default: {
            return get_type_size(type);
        }
    }
}

enum struct_classes {
    CLASS_MEMORY = 0,
    CLASS_INTEGER = 1,
};

typedef bool struct_classes_t;

operand_t get_register_from_arg_num(uint8_t num){
    switch(num){
        case 0:
            return REG_DI;
        case 1:
            return REG_SI;
        case 2:
            return REG_DX;
        case 3:
            return REG_CX;
        case 4:
            return REG_R8;
        case 5:
            return REG_R9;
        default:
            return OP_NULL;
    }
}

/*
* generates the function arguments for the caller, sets up all values
*/
size_t generate_func_call_args(node_t* nod_args, size_t return_size){
    ir_node_t ir_node;

    ir_operand_t* stack_push = alloc_array(sizeof(ir_operand_t), 1);;

    uint8_t reg_num;
    ir_operand_t return_val;

    if ((reg_num = return_size > 16 ? 1 : 0)){
        return_val = var_gen(return_size);
        ir_node = (ir_node_t){
            .instruction = INST_GET_ADDRESS,
            .op_1 = return_val,
            .dst.type = REG_DI,
            .size = 8
        };
        pback_array(&ir_code, &ir_node);
    }
    else
        return_val.type = REG_AX;

    for (size_t i = 0LU; i < get_count_array(nod_args); ++i){
        ir_gen_node(nod_args[i]);

        size_t t_size = get_type_size(nod_args[i].d_type);

        /* pass it using legacy methods */
        if (t_size <= 8){
            if (reg_num < 5){
                ir_operand_t dst;

                dst.type = get_register_from_arg_num(reg_num++);

                ir_node = (ir_node_t){
                    .instruction = INST_COPY,
                    .dst = dst,
                    .op_1 = value,
                    .size = t_size
                };
                pback_array(&ir_code, &ir_node);
            }   
            else {
                /* move the value into an l(ish)value */
                if (value.type == MEM_ADDRESS || value.type < ___IMMEDIATE_TYPE_START___){
                    ir_node = (ir_node_t){
                        .instruction = INST_COPY,
                        .op_1 = value,
                        .dst = var_gen(t_size),
                        .size = t_size
                    };
                    pback_array(&ir_code, &ir_node);
                    value = ir_node.dst;
                }

                pback_array(&stack_push, &value);
            }
        }
        /* pass it using more complex methods */
        else {
            if (t_size <= 16){
                if (reg_num < 4){
                    ir_node = (ir_node_t){
                        .instruction = INST_COPY,
                        .dst.type = get_register_from_arg_num(reg_num++),
                        .op_1 = value,
                        .size = 8
                    };
                    pback_array(&ir_code, &ir_node);

                    ir_node = (ir_node_t){
                        .instruction = INST_COPY,
                        .dst.type = get_register_from_arg_num(reg_num++),
                        .op_1 = get_val_offset(value, 8),
                        .size = t_size - 8
                    };
                    pback_array(&ir_code, &ir_node);
                }
                else {
                    if (value.type == MEM_ADDRESS || value.type < ___IMMEDIATE_TYPE_START___){
                        ir_node = (ir_node_t){
                            .instruction = INST_COPY,
                            .op_1 = value,
                            .dst = var_gen(t_size),
                            .size = t_size
                        };
                        pback_array(&ir_code, &ir_node);
                        value = ir_node.dst;
                    }

                    pback_array(&stack_push, &value);
                }
            }
            else {
                ir_operand_t base_val = value;
                for (size_t byte_num = 0; byte_num < t_size; byte_num += 8){
                    value = get_val_offset(base_val, byte_num);
                    pback_array(&stack_push, &value);
                }
            }
        }
    }

    size_t stack_count = get_count_array(stack_push);

    if (stack_count){
        for (size_t i = 1; i <= stack_count; ++i){
            ir_node = (ir_node_t){
                .instruction = INST_STACK_PUSH,
                .op_1 = stack_push[stack_count - i],
                .size = 8
            };
            pback_array(&ir_code, &ir_node);
        }
    }

    free_array(stack_push, NULL);

    value = return_val;

    return stack_count * 8;
}

/*
* generates the function argument values for the callee,
* moves register arguments into memory
*/
ir_operand_t* generate_func_args(symbol_t* sym_args, size_t return_size){
    size_t num_eightbytes = 0;
    ir_operand_t* args = alloc_array(sizeof(ir_operand_t), 1);
    ir_node_t ir_node;

    uint8_t reg_num = return_size > 16 ? 1 : 0;

    for (size_t arg_count = 0; arg_count < get_count_array(sym_args); ++arg_count){
        size_t t_size = get_type_size(sym_args[arg_count].data_type);

        if (t_size <= 8){
            if (reg_num < 5){
                ir_node = (ir_node_t){
                    .instruction = INST_COPY,
                    .op_1 = get_register_from_arg_num(reg_num++),
                    .dst = var_gen(t_size),
                    .size = t_size
                };
                pback_array(&ir_code, &ir_node);

                pback_array(&args, &ir_node.dst);
            }
            else {
                value.type = STK_OFFSET;
                value.offset = 8 * num_eightbytes++ + 16;

                pback_array(&args, &value);
            }
        }
        else {
            if (t_size <= 16){
                if (reg_num < 4){
                    ir_operand_t dst = var_gen(t_size);

                    ir_node = (ir_node_t){
                        .instruction = INST_COPY,
                        .op_1.type = get_register_from_arg_num(reg_num++),
                        .dst = dst,
                        .size = 8
                    };
                    pback_array(&ir_code, &ir_node);

                    ir_node = (ir_node_t){
                        .instruction = INST_COPY,
                        .op_1.type = get_register_from_arg_num(reg_num++),
                        .dst = get_val_offset(dst, 8),
                        .size = t_size - 8
                    };
                    pback_array(&ir_code, &ir_node);

                    pback_array(&args, &dst);
                }
                else {
                    value.type = STK_OFFSET;
                    value.offset = 8 * num_eightbytes + 16;

                    num_eightbytes += 2;

                    pback_array(&args, &value);
                }
            }
            else {
                value.type = STK_OFFSET;
                value.offset = 8 * num_eightbytes + 16;
                num_eightbytes += t_size / 8;

                if (t_size % 8)
                    ++num_eightbytes;

                pback_array(&args, &value);
            }
        }
    }

    return args;
}

/* returns a value with the offset applied */
ir_operand_t get_val_offset(ir_operand_t val, long offset){
    if (val.type == STK_OFFSET){
        val.offset += offset;
    } else
    if (val.type == STATIC_MEM){
        val.secondary_offset = offset;
        val.type = STATIC_MEM_OFFSET;
    } else
    if (val.type == STATIC_MEM_LOCAL){
        val.secondary_offset = offset;
        val.type = STATIC_MEM_LOCAL_OFFSET;
    } else
    if (val.type == STATIC_MEM_OFFSET){
        val.secondary_offset += offset;
    } else
    if (val.type == STATIC_MEM_LOCAL_OFFSET){
        val.secondary_offset += offset;
    } else
    if (val.type == MEM_ADDRESS){
        val.type = MEM_ADDRESS_OFFSET;
        val.secondary_offset = offset;
    }

    return val;
}

