#include "ir_gen.h"

#include <stdio.h>

size_t frame_size;

static ir_func_t ir_gen_func(nod_function_t node);
static ir_node_t* ir_code;

static ir_node_t null_node = (ir_node_t){
    .instruction = INST_NOP
};

#define var_gen(size)                      \
    (frame_size += (size), (ir_operand_t){ \
        .type = STK_OFFSET,                \
        .offset = -(signed long)frame_size \
    })

ir_func_t* ir_gen(node_t* program_ast){
    ir_func_t* program = alloc_array(sizeof(ir_func_t), 1);
    
    for (size_t i = 0; i < get_size_array(program_ast); ++i){
        node_t* node = program_ast + i;
        if (node->type == NOD_FUNC){
            ir_func_t func = ir_gen_func(node->func_node);
            pback_array(&program, &func);
        }
        else {
            fprintf(stderr, "Expected function as top-level statement\n");
            exit(-1);
        }
    }

    return program;
}

static void ir_gen_node(node_t node);

static ir_func_t ir_gen_func(nod_function_t node){
    ir_code = alloc_array(sizeof(ir_node_t), 1);
    
    ir_gen_node((node_t){
        .type = NOD_BLOCK,
        .block_node = node.body
    });

    return (ir_func_t){
        .name = node.name,
        .code = ir_code,
        .frame_size = frame_size
    };
}

static ir_operand_t value = (ir_operand_t){.type = OP_NULL};

void ir_gen_binary_op(ir_inst_t inst, node_t* a, node_t* b){
    ir_gen_node(*a);
    ir_operand_t val_a = value;

    ir_gen_node(*b);

    ir_node_t ir_node = (ir_node_t){
        .instruction = inst,
        .op_1 = val_a,
        .op_2 = value,
        .dst = var_gen(4)
    };
    pback_array(&ir_code, &ir_node);

    value = ir_node.dst;
}

static void ir_gen_node(node_t node){
    static ir_node_t ir_node = (ir_node_t){.instruction = INST_NOP};
    /* all labels are 'L%lu' */
    static size_t label_count = 0;

    switch(node.type){
        case NOD_NULL: {
            perror("unexpected node");
            exit(-1);
        }

        case NOD_INTEGER: {
            value = (ir_operand_t){
                .type = IMM_U32,
                .immediate = node.int_node.val
            };

            return;
        }

        case NOD_RETURN: {

            ir_gen_node(*node.return_node.value);

            ir_node = (ir_node_t){
                .instruction = INST_RET,
                .op_1 = value
            };
            pback_array(&ir_code, &ir_node);
            return;
        }

        case NOD_BLOCK: {
            size_t arr_size = get_size_array(node.block_node.code);
            for (size_t i = 0; i < arr_size; ++i)
                ir_gen_node(node.block_node.code[i]);
            return;
        }

        case NOD_UNARY_SUB: {
            ir_gen_node(*node.unary_node.value);

            ir_node = (ir_node_t){
                .dst = var_gen(4),
                .instruction = INST_NEGATE,
                .op_1 = value
            };
            pback_array(&ir_code, &ir_node);

            value = ir_node.dst;
            return;
        }

        case NOD_BITWISE_NOT: {
            ir_gen_node(*node.unary_node.value);

            ir_node = (ir_node_t){
                .dst = var_gen(4),
                .instruction = INST_BITWISE_NOT,
                .op_1 = value
            };
            pback_array(&ir_code, &ir_node);

            value = ir_node.dst;
            return;
        }

        case NOD_LOGICAL_NOT: {
            ir_gen_node(*node.unary_node.value);

            ir_node = (ir_node_t){
                .instruction = INST_LOGICAL_NOT,
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
            ir_gen_binary_op(INST_DIV, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }

        case NOD_BINARY_MOD: {
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
                .op_1 = value,
                .op_2.label_id = false_label
            };
            pback_array(&ir_code, &ir_node);

            ir_gen_node(*node.binary_node.value_b);
            ir_node = (ir_node_t){
                .instruction = INST_JUMP_IF_Z,
                .op_1 = value,
                .op_2.label_id = false_label
            };
            pback_array(&ir_code, &ir_node);

            ir_node = (ir_node_t){
                .instruction = INST_COPY,
                .dst = dst,
                .op_1.type = IMM_U32, .op_1.immediate = 1
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
                .op_1.type = IMM_U32, .op_1.immediate = 0
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
                .dst.label_id = true_label,
                .op_1 = value
            };
            pback_array(&ir_code, &ir_node);

            ir_gen_node(*node.binary_node.value_b);

            ir_node = (ir_node_t){
                .instruction = INST_JUMP_IF_NZ,
                .dst.label_id = true_label,
                .op_1 = value
            };
            pback_array(&ir_code, &ir_node);

            ir_node = (ir_node_t){
                .instruction = INST_COPY,
                .dst = dst,
                .op_1   .type = IMM_U32,
                .op_1   .immediate = 0
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
                .op_1   .immediate = 1
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
            ir_gen_binary_op(INST_SET_E, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }
        
        case NOD_NOT_EQUAL_TO: {
            ir_gen_binary_op(INST_SET_NE, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }
        
        case NOD_GREATER_THAN: {
            ir_gen_binary_op(INST_SET_G, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }
        
        case NOD_LESS_THAN: {
            ir_gen_binary_op(INST_SET_L, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }
        
        case NOD_GREATER_THAN_OR_EQUAL: {
            ir_gen_binary_op(INST_SET_GE, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }
        
        case NOD_LESS_THAN_OR_EQUAL: {
            ir_gen_binary_op(INST_SET_LE, node.binary_node.value_a, node.binary_node.value_b);
            return;
        }

        default: {
            perror("unexpected expression");
            exit(-1);
        }
    }
}

void ir_node_dstr(void* _node){
    ir_func_t* node = _node;
    free(node->name);
    free_array(node->code, NULL);
}