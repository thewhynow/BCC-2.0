#include "code_gen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
    #define ASM_FUNCTION_PREFIX "_"
#endif

#ifdef __linux__
    #define ASM_FUNCTION_PREFIX ""
#endif

static FILE* asm_file;

void codegen_func(ir_func_t func);
void codegen_node(ir_node_t node);

void codegen(ir_func_t* nodes, const char* asm_file_path){
    asm_file = fopen(asm_file_path, "w");
    for (size_t i = 0; i < get_size_array(nodes); ++i)
        codegen_func(nodes[i]);

    #ifdef __linux__
        fprintf(asm_file, ".section .note.GNU-stack, \"\",@progbits\n");
    #endif
}

void codegen_func(ir_func_t func){
    fprintf(asm_file, 
        ".globl " ASM_FUNCTION_PREFIX "%s\n"
        ASM_FUNCTION_PREFIX "%s:\n"
        "pushq %%rbp\n"
        "movq %%rsp, %%rbp\n"
        "subq $%lu, %%rsp\n",
        func.name, func.name, func.frame_size);

    for (size_t i = 0; i < get_size_array(func.code); ++i)
        codegen_node(func.code[i]);

    fprintf(asm_file,
        "movq %%rbp, %%rsp\n"
        "popq %%rbp\n"
        "ret\n");
}

const char* codegen_val(ir_operand_t val){
    static char str_buff[4096];
    switch(val.type){
        case REG_AX:
            return "%eax";
        case REG_BX:
            return "%ebx";
        case REG_CX:
            return "%ecx";
        case REG_DX:
            return "%edx";
        case REG_SI:
            return "%esi";
        case REG_DI:
            return "%edi";
        case REG_BP:
            return "%rbp";
        case REG_SP:
            return "%rsp";
        case REG_R8:
            return "%r8d";
        case REG_R9:
            return "%r9d";
        case REG_R10:
            return "%r10d";
        case REG_R11:
            return "%r11d";
        case REG_R12:
            return "%r12d";
        case REG_R13:
            return "%r13d";
        case REG_R14:
            return "%r14d";
        case REG_R15:
            return "%r15d";
        case STK_OFFSET:
            snprintf(str_buff, 4096, "%ld(%%rbp)", val.offset);
            return str_buff;
        case IMM_U32:
            snprintf(str_buff, 4096, "$%u", val.immediate);
            return str_buff;
        default:
            return NULL;
    }
}

void codegen_mov(ir_operand_t src, ir_operand_t dst){
    static char alt_val_buff[4096];
    
    strcpy(alt_val_buff, codegen_val(src));
    
    if (src.type > ___LVALUE_TYPE_START___ && dst.type > ___LVALUE_TYPE_START___){
        fprintf(asm_file,
        "movl %s, %%eax\n"
        "movl %%eax, %s\n"
        , alt_val_buff, codegen_val(dst));
    }
    else
        fprintf(asm_file, 
            "movl %s, %s\n", 
            alt_val_buff, codegen_val(dst));
}

void codegen_binary(const char* inst, ir_operand_t a, ir_operand_t b, ir_operand_t dst){
    codegen_mov(a, dst);

    if (b.type > ___LVALUE_TYPE_START___ && dst.type > ___LVALUE_TYPE_START___){
        codegen_mov(b, (ir_operand_t){.type = REG_R10});
        
        fprintf(asm_file,
        "%s %%r10d, %s\n",
        inst, codegen_val(dst));
    }
    else {
        static char b_val_str[4096];

        strncpy(b_val_str, codegen_val(b), 4096);

        fprintf(asm_file,
        "%s %s, %s\n",
        inst, b_val_str, codegen_val(dst));
    }
}

void codegen_comp(ir_node_t node){
    const char* comp_inst = NULL;
    switch(node.instruction){
        case INST_SET_E:
            comp_inst = "e";
            break;
        case INST_SET_NE:
            comp_inst = "ne";
            break;
        case INST_SET_G:
            comp_inst = "g";
            break;
        case INST_SET_L:
            comp_inst = "l";
            break;
        case INST_SET_GE:
            comp_inst = "ge";
            break;
        case INST_SET_LE:
            comp_inst = "le";
            break;
        default:
            printf(stderr, "The fact that you got this error message is honestly amazing. I give my props to you, fellow coder. \n");
            exit(-1);
    }
/*
    maybe one day ...

    const char* comp_inst = 
        node.instruction == INST_SET_E ?
            "e":
        node.instruction == INST_SET_NE ?
            "ne":
        node.instruction == INST_SET_G ?
            "g":
        node.instruction == INST_SET_L ?
            "l":
        node.instruction == INST_SET_GE ?
            "ge":
        node.instruction == INST_SET_LE ?
            "le":
        NULL;
*/

    codegen_binary("cmpl", node.op_1, node.op_2, node.dst);

    fprintf(asm_file,
    "set%s %s\n",
    comp_inst, codegen_val(node.dst));
}

void codegen_node(ir_node_t node){
    switch(node.instruction){
        case INST_NOP:{ 
            return;
        }

        case INST_RET:{
            codegen_mov(node.op_1, (ir_operand_t){
                .type = REG_AX
            });
            fprintf(asm_file,
                "movq %%rbp, %%rsp\n"
                "popq %%rbp\n"
                "ret\n");
            return;
        }
        
        case INST_NEGATE:{
            codegen_mov(node.op_1, (ir_operand_t){.type = REG_AX});
            fprintf(asm_file,
            "negl %%eax\n"
            "movl %%eax, %s\n"
            , codegen_val(node.dst));
            return;
        }

        case INST_BITWISE_NOT: {
            codegen_mov(node.op_1, (ir_operand_t){.type = REG_AX});
            fprintf(asm_file,
            "notl %%eax\n"
            "movl %%eax, %s\n"
            , codegen_val(node.dst));
            return;
;        }

        case INST_ADD: {
            codegen_binary("addl", node.op_1, node.op_2, node.dst);
            return;
        }

        case INST_SUB: {
            codegen_binary("subl", node.op_1, node.op_2, node.dst);
            return;
        }

        case INST_MUL: {
            codegen_mov(node.op_2, (ir_operand_t){.type = REG_R10});
            
            if (node.op_1.type == IMM_U32){
                codegen_mov(node.op_1, (ir_operand_t){.type = REG_R11});
                fprintf(asm_file,
                "imull %%r11d, %%r10d\n"
                );
            }
            else {
                fprintf(asm_file,
                "imull %s, %%r10d\n",
                codegen_val(node.op_1));
            }

            codegen_mov((ir_operand_t){.type = REG_R10}, node.dst);

            return;
        }

        case INST_DIV: {
            codegen_mov(node.op_1, (ir_operand_t){.type = REG_AX});
            if (is_immediate(node.op_2.type)){
                fprintf(asm_file,
                "cdq\n"
                "movl $%u, %%r10d\n"
                "idivl %%r10d\n"
                "movl %%eax, %s\n", node.op_2.immediate, codegen_val(node.dst));
            }
            else {
                static char val_str_buff[4096];

                strncpy(val_str_buff, codegen_val(node.op_2), 4096);

                fprintf(asm_file,
                "cdq\n"
                "idivl %s\n"
                "movl %%eax, %s\n",
                val_str_buff, codegen_val(node.dst));
            }
            return;
        }

        case INST_MOD: {
            codegen_mov(node.op_1, (ir_operand_t){.type = REG_AX});
            if (is_immediate(node.op_2.type)){
                fprintf(asm_file,
                "cdq\n"
                "movl $%u, %%r10d\n"
                "idivl %%r10d\n"
                "movl %%edx, %s\n", node.op_2.immediate, codegen_val(node.dst));
            }
            else {
                static char val_str_buff[4096];

                strncpy(val_str_buff, codegen_val(node.op_2), 4096);

                fprintf(asm_file,
                "cdq\n"
                "idivl %s\n"
                "movl %%edx, %s\n",
                val_str_buff, codegen_val(node.dst));
            }
            return;
        }

        case INST_BITWISE_AND: {
            codegen_binary("andl", node.op_1, node.op_2, node.dst);
            return;
        }
        
        case INST_BITWISE_OR: {
            codegen_binary("orl", node.op_1, node.op_2, node.dst);
            return;
        }

        case INST_BITWISE_XOR: {
            codegen_binary("xorl", node.op_1, node.op_2, node.dst);
            return;
        }
        
        case INST_SHIFT_LEFT: {
            codegen_binary("shll", node.op_1, node.op_2, node.dst);
            return;
        }
        
        case INST_SHIFT_RIGHT: {
            codegen_binary("shrl", node.op_1, node.op_2, node.dst);
            return;
        }

        case INST_COPY: {
            codegen_mov(node.op_1, node.dst);
            return;
        }

        case INST_JUMP: {
            fprintf(asm_file,
            "jmp L%lu\n",
            node.dst.label_id);
            return;
        }

        case INST_JUMP_IF_Z: {
            if (is_immediate(node.op_1.type)){
                codegen_mov(node.op_1, (ir_operand_t){.type = REG_R10});
                fprintf(asm_file, 
                "cmpl $0, %%r10d\n"
                "je L%lu\n",
                node.dst.label_id);
            }
            else {
                fprintf(asm_file, 
                "cmpl $0, %s\n"
                "je L%lu\n",
                codegen_val(node.op_1), node.dst.label_id);
            }
            return;
        }

        case INST_JUMP_IF_NZ: {
            if (is_immediate(node.op_1.type)){
                codegen_mov(node.op_1, (ir_operand_t){.type = REG_R10});
                fprintf(asm_file, 
                "cmpl $0, %%r10d\n"
                "jne L%lu\n",
                node.dst.label_id);
            }
            else {
                fprintf(asm_file, 
                "cmpl $0, %s\n"
                "jne L%lu\n",
                codegen_val(node.op_1), node.dst.label_id);
            }
            return;
        }

        case INST_LABEL: {
            fprintf(asm_file,
            "L%lu:\n",
            node.dst.label_id);
            return;
        }

        default: {
            puts("expression is not supported yet");
            exit(-1);
        }

    }
}