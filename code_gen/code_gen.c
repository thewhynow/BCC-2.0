#include "code_gen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __APPLE__
    #define ASM_SYMBOL_PREFIX "_"
#endif

#ifdef __linux__
    #define ASM_SYMBOL_PREFIX ""
#endif

static FILE* asm_file;
static char* func_name;
static size_t func_num = -1;

void codegen_func(ir_node_t func, storage_class_t storage_class);
void codegen_node(ir_node_t node);

#define asm_inst_prefix(size)\
    ((size) == 4 ? 'l' : (size) == 8 ? 'q' : (size) == 2 ? 'w' : (size) == 1 ? 'b' : 'E')

#define asm_static_directive(size)\
    ((size) == 4 ? "long" : (size) == 8 ? "quad" : (size) == 2 ? "word" : (size) == 1 ? "byte" : "ASM STATIC DIRECTIVE ERROR")

char* strdup(const char* s);

const char* generate_asm_inst(const char* inst, size_t size){
    static char buff[4096];
    snprintf(buff, 4096, "%s%c", inst, asm_inst_prefix(size));
    return buff;
}

void codegen(ir_node_t* nodes, const char* asm_file_path){
    asm_file = fopen(asm_file_path, "w");
    
    fprintf(asm_file, ".text\n");

    for (size_t i = 0; i < get_count_array(nodes); ++i)
        codegen_node(nodes[i]);

    #ifdef __linux__
        fprintf(asm_file, ".section .note.GNU-stack, \"\",@progbits\n");
    #endif

    fclose(asm_file);
}

void codegen_func(ir_node_t func, storage_class_t storage_class){
    func_name = func.dst.identifier;
    ++func_num;

    if (storage_class != STORAGE_STATIC)
        fprintf(asm_file, ".globl " ASM_SYMBOL_PREFIX "%s\n", func.dst.identifier);
    fprintf(asm_file, 
        ASM_SYMBOL_PREFIX "%s:\n"
        "pushq %%rbp\n"
        "movq %%rsp, %%rbp\n"
        "subq $%lu, %%rsp\n",
        func.dst.identifier, func.op_1.label_id);
    
}

static const char* REG_NAMES[][4];

const char* codegen_val(ir_operand_t val, size_t size){
    static char str_buff[4096];
    switch(val.type){
        case REG_AX:
        case REG_BX:
        case REG_CX:
        case REG_DX:
        case REG_SI:
        case REG_DI:
        case REG_BP:
        case REG_SP:
        case REG_R8:
        case REG_R9:
        case REG_R10:
        case REG_R11:
        case REG_R12:
        case REG_R13:
        case REG_R14:
        case REG_R15:
            return REG_NAMES[val.type - 1][(long)ceill(log2l(size))];

        case STK_OFFSET:
            snprintf(str_buff, 4096, "%ld(%%rbp)", val.offset);
            return str_buff;
        case IMM_U32:
        case IMM_U64:
        case IMM_U16:
        case IMM_U8:
            snprintf(str_buff, 4096, "$%lu", val.immediate);
            return str_buff;
        case STATIC_MEM:
            snprintf(str_buff, 4096, "%s(%%rip)", val.identifier);
            return str_buff;
        case STATIC_MEM_LOCAL:
            snprintf(str_buff, 4096, ASM_SYMBOL_PREFIX ".%lu_%s_%s(%%rip)", func_num, func_name, val.identifier);
            return str_buff;
        default:
            return NULL;
    }
}

void codegen_mov(ir_operand_t src, ir_operand_t dst, size_t size){
    static char alt_val_buff[4096];
    
    strcpy(alt_val_buff, codegen_val(src, size));

    char* ax_str = strdup(codegen_val((ir_operand_t){.type = REG_AX}, size));
    
    if (src.type > ___LVALUE_TYPE_START___ && dst.type > ___LVALUE_TYPE_START___){
        fprintf(asm_file,
        "mov%1$c %2$s, %3$s\n"
        "mov%1$c %3$s, %4$s\n",
        asm_inst_prefix(size), alt_val_buff, ax_str, codegen_val(dst, size));
    }
    else
        fprintf(asm_file, 
            "%s %s, %s\n", 
            generate_asm_inst("mov", size), alt_val_buff, codegen_val(dst, size));
}

void codegen_binary(const char* inst, ir_operand_t a, ir_operand_t b, ir_operand_t dst, size_t size){
    char inst_suffix = asm_inst_prefix(size);
    
    if (dst.type != OP_NULL){
        codegen_mov(a, dst, size);

        if (b.type > ___LVALUE_TYPE_START___ && dst.type > ___LVALUE_TYPE_START___){
            codegen_mov(b, (ir_operand_t){.type = REG_R10}, size);

            /* this is newer code so you see some better practices - im not going to bother rewriting old code */

            char* str = strdup(codegen_val((ir_operand_t){.type = REG_R10}, size));

            fprintf(asm_file,
            "%s%c %s, %s\n",
            inst, inst_suffix, str, codegen_val(dst, size));

            free(str);
        }
        else {
            static char b_val_str[4096];

            strncpy(b_val_str, codegen_val(b, size), 4096);

            fprintf(asm_file,
            "%s%c %s, %s\n",
            inst, inst_suffix, b_val_str, codegen_val(dst, size));
        }
    }
    else {
        if (b.type > ___LVALUE_TYPE_START___ && dst.type > ___LVALUE_TYPE_START___){
            codegen_mov(b, (ir_operand_t){.type = REG_R10}, size);

            char* str = strdup(codegen_val((ir_operand_t){.type = REG_R10}, size));

            fprintf(asm_file,
            "%s%c %s, %s\n",
            inst, inst_suffix, str, codegen_val(a, size));

            free(str);
        }
        else {
            static char b_val_str[4096];

            strncpy(b_val_str, codegen_val(b, size), 4096);

            fprintf(asm_file,
            "%s%c %s, %s\n",
            inst, inst_suffix, b_val_str, codegen_val(a, size));
        }
    }
    
}

void codegen_comp(ir_node_t node){
    const char* comp_inst;
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
        case INST_SET_A:
            comp_inst = "a";
            break;
        case INST_SET_AE:
            comp_inst = "ae";
            break;
        case INST_SET_B:
            comp_inst = "b";
            break;
        case INST_SET_BE:
            comp_inst = "be";
            break;
        default:
            fprintf(stderr, "The fact that you got this error message is honestly amazing. I give my props to you, fellow coder. \n");
            exit(-1);
    }

    codegen_binary("cmp", node.op_1, node.op_2, (ir_operand_t){.type = OP_NULL}, node.size);

    fprintf(asm_file,
    "movl $0, %1$s\n"
    "set%2$s %1$s\n",
    codegen_val(node.dst, 4), comp_inst);
}

void codegen_node(ir_node_t node){
    switch(node.instruction){
        char* str;

        case INST_NOP:{ 
            return;
        }

        case INST_LOGICAL_NOT: {
            str = strdup(codegen_val(node.op_1, node.size));

            fprintf(asm_file,
            "cmp%c $0, %s\n"
            "sete %s\n",
            asm_inst_prefix(node.size), str, codegen_val(node.dst, INST_TYPE_LONGWORD));

            return;
        }

        case INST_RET:{
            codegen_mov(node.op_1, (ir_operand_t){
                .type = REG_AX
            }, node.size);
            fprintf(asm_file,
                "movq %%rbp, %%rsp\n"
                "popq %%rbp\n"
                "ret\n");
            return;
        }
        
        case INST_NEGATE:{
            codegen_mov(node.op_1, (ir_operand_t){.type = REG_AX}, node.size);

            str = strdup(codegen_val((ir_operand_t){.type = REG_AX}, node.size));

            fprintf(asm_file,
            "neg%1$c %2$s\n"
            "mov%1$c %2$s, %3$s\n",
            asm_inst_prefix(node.size), str, codegen_val(node.dst, node.size));
            return;
        }

        case INST_BITWISE_NOT: {
            str = strdup(codegen_val(node.dst, node.size));

            codegen_mov(node.op_1, (ir_operand_t){.type = REG_AX}, node.size);
            fprintf(asm_file,
            "not%1$c %2$s\n"
            "mov%1$c %2$s, %3$s\n",
            asm_inst_prefix(node.size), codegen_val((ir_operand_t){.type = REG_R10}, node.size), str);

            free(str);

            return;
        }

        case INST_ADD: {
            codegen_binary("add", node.op_1, node.op_2, node.dst, node.size);
            return;
        }

        case INST_SUB: {
            codegen_binary("sub", node.op_1, node.op_2, node.dst, node.size);
            return;
        }

        case INST_MUL: {
            codegen_mov(node.op_2, (ir_operand_t){.type = REG_R10}, node.size);
            
            if (node.op_1.type == IMM_U32 || node.op_1.type == IMM_U64){
                codegen_mov(node.op_1, (ir_operand_t){.type = REG_R11}, node.size);
                
                str = strdup(codegen_val((ir_operand_t){.type = REG_R11}, node.size));

                fprintf(asm_file,
                "imul%c %s, %s\n",
                asm_inst_prefix(node.size), str, codegen_val((ir_operand_t){.type = REG_R10}, node.size));

                free(str);
            }
            else {
                str = strdup(codegen_val((ir_operand_t){.type = REG_R10}, node.size));

                fprintf(asm_file,
                "imul%c %s, %s\n",
                asm_inst_prefix(node.size), codegen_val(node.op_1, node.size), str);

                free(str);
            }

            codegen_mov((ir_operand_t){.type = REG_R10}, node.dst, node.size);

            return;
        }

        case INST_DIV: {
            codegen_mov(node.op_1, (ir_operand_t){.type = REG_AX}, node.size);
            if (is_immediate(node.op_2.type)){
                str = strdup(codegen_val((ir_operand_t){.type = REG_AX}, node.size));

                const char* other_str = strdup(codegen_val((ir_operand_t){.type = REG_R10}, node.size));
                
                fprintf(asm_file,
                "cdq\n"
                "mov%1$c $%2$lu, %3$s\n"
                "idiv%1$c %3$s\n"
                "mov%1$c %4$s, %5$s\n",
                asm_inst_prefix(node.size), node.op_2.immediate, other_str, str, codegen_val(node.dst, node.size));
            }
            else {
                str = strdup(codegen_val(node.op_2, node.size));

                const char* other_str = strdup(codegen_val((ir_operand_t){.type = REG_AX}, node.size));

                fprintf(asm_file,
                "cdq\n"
                "idiv%1$c %2$s\n"
                "mov%1$c %3$s, %4$s\n",
                asm_inst_prefix(node.size), str, other_str, codegen_val(node.dst, node.size));
            }
            return;
        }

        case INST_MOD: {
            codegen_mov(node.op_1, (ir_operand_t){.type = REG_AX}, node.size);
            char* edx_str = strdup(codegen_val((ir_operand_t){.type = REG_DX}, node.size));

            if (is_immediate(node.op_2.type)){
                str = strdup(codegen_val((ir_operand_t){.type = REG_R10}, node.size));

                fprintf(asm_file,
                "cdq\n"
                "mov%1$c $%2$lu, %3$s\n"
                "idiv%1$c %3$s\n"
                "mov%1$c %4$s, %5$s\n", 
                asm_inst_prefix(node.size), node.op_2.immediate, str, edx_str, codegen_val(node.dst, node.size));

                free(str);
            }
            else {
                char* other_str = strdup(codegen_val(node.op_2, node.size));

                fprintf(asm_file,
                "cdq\n"
                "idiv%1$c %2$s\n"
                "mov%1$c %3$s, %4$s\n",
                asm_inst_prefix(node.size), other_str, edx_str, codegen_val(node.dst, node.size));

                free(other_str);
            }
            free(edx_str);
            return;
        }

        case INST_BITWISE_AND: {
            codegen_binary("and", node.op_1, node.op_2, node.dst, node.size);
            return;
        }
        
        case INST_BITWISE_OR: {
            codegen_binary("or", node.op_1, node.op_2, node.dst, node.size);
            return;
        }

        case INST_BITWISE_XOR: {
            codegen_binary("xor", node.op_1, node.op_2, node.dst, node.size);
            return;
        }
        
        case INST_SHIFT_LEFT: {
            codegen_binary("shl", node.op_1, node.op_2, node.dst, node.size);
            return;
        }
        
        case INST_SHIFT_RIGHT: {
            codegen_binary("shr", node.op_1, node.op_2, node.dst, node.size);
            return;
        }

        case INST_COPY: {
            codegen_mov(node.op_1, node.dst, node.size);
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
                codegen_mov(node.op_1, (ir_operand_t){.type = REG_R10}, node.size);
                fprintf(asm_file, 
                "cmp%c $0, %s\n"
                "je L%lu\n",
                asm_inst_prefix(node.size), codegen_val((ir_operand_t){.type = REG_R10}, node.size), node.dst.label_id);
            }
            else {
                fprintf(asm_file, 
                "cmp%c $0, %s\n"
                "je L%lu\n",
                asm_inst_prefix(node.size), codegen_val(node.op_1, node.size), node.dst.label_id);
            }
            return;
        }

        case INST_JUMP_IF_NZ: {
            if (is_immediate(node.op_1.type)){
                codegen_mov(node.op_1, (ir_operand_t){.type = REG_R10}, node.size);
                fprintf(asm_file, 
                "cmp%c $0, %s\n"
                "jne L%lu\n",
                asm_inst_prefix(node.size), codegen_val((ir_operand_t){.type = REG_R10}, node.size), node.dst.label_id);
            }
            else {
                fprintf(asm_file, 
                "cmp%c $0, %s\n"
                "jne L%lu\n",
                asm_inst_prefix(node.size), codegen_val(node.op_1, node.size), node.dst.label_id);
            }
            return;
        }

        case INST_LABEL: {
            fprintf(asm_file,
            "L%lu:\n",
            node.dst.label_id);
            return;
        }

        case INST_SET_A:
        case INST_SET_AE:
        case INST_SET_B:
        case INST_SET_BE:
        case INST_SET_E:
        case INST_SET_NE:
        case INST_SET_G:
        case INST_SET_GE:
        case INST_SET_L:
        case INST_SET_LE: {
            codegen_comp(node);
            return;
        }

        case INST_INCREMENT: {
            fprintf(asm_file,
            "inc%c %s\n",
            asm_inst_prefix(node.size), codegen_val(node.dst, node.size));
            return;
        }

        case INST_USER_LABEL: {
            fprintf(asm_file,
            "%s:\n",
            node.dst.identifier);
            return;
        }

        case INST_GOTO: {
            fprintf(asm_file,
            "jmp %s\n",
            node.dst.identifier);
            return;
        }

        case INST_JUMP_IF_EQ: {
            codegen_binary("cmp", node.op_1, node.op_2, (ir_operand_t){.type = OP_NULL}, node.size);
            fprintf(asm_file,
            "je L%lu\n",
            node.dst.label_id);
            return;
        }

        case INST_STACK_ALLOC: {
            fprintf(asm_file,
            "subq %s, %%rsp\n",
            codegen_val(node.op_1, node.size));
            return;
        }

        case INST_STACK_DEALLOC: {
            fprintf(asm_file,
            "addq %s, %%rsp\n",
            codegen_val(node.op_1, node.size));
            return;
        }

        case INST_STACK_PUSH: {
            fprintf(asm_file,
            "pushq %s\n",
            codegen_val(node.op_1, node.size));
            return;
        }

        case INST_CALL: {
            fprintf(asm_file,
            "call " ASM_SYMBOL_PREFIX "%s\n",
            node.dst.identifier);
            return;
        }

        case INST_PUBLIC_FUNC: {
            codegen_func(node, STORAGE_NULL);
            return;
        }

        case INST_STATIC_FUNC: {
            codegen_func(node, STORAGE_STATIC);
            return;
        }

        case INST_INIT_STATIC: {
            fprintf(asm_file,
            ".data\n"
            ".balign %lu\n"
            ASM_SYMBOL_PREFIX "%s:\n"
            ".%s %lu\n"
            ".text\n",
            node.op_2.label_id, node.dst.identifier, asm_static_directive(node.op_2.label_id), node.op_1.immediate);
            return;
        }

        case INST_INIT_STATIC_Z: {
            fprintf(asm_file,
            ".bss\n"
            ".balign %lu\n"
            ASM_SYMBOL_PREFIX "%s:\n"
            ".zero %lu\n"
            ".text\n",
            node.op_2.label_id, node.dst.identifier, node.op_2.label_id);
            return;
        }

        case INST_INIT_STATIC_PUBLIC: {
            fprintf(asm_file,
            ".globl " ASM_SYMBOL_PREFIX "%s\n"
            ".data\n"
            ".balign %lu\n"
            ASM_SYMBOL_PREFIX "%s:\n"
            ".%s %lu\n"
            ".text\n",
            node.dst.identifier, node.op_2.label_id, node.dst.identifier, asm_static_directive(node.size), node.op_1.immediate);
            return;
        }

        case INST_INIT_STATIC_Z_PUBLIC: {
            fprintf(asm_file,
            ".globl " ASM_SYMBOL_PREFIX "%s\n"
            ".bss\n"
            ".balign %lu\n"
            ASM_SYMBOL_PREFIX "%s:\n"
            ".zero %lu\n"
            ".text\n",
            node.dst.identifier, node.op_2.label_id, node.dst.identifier, node.op_2.label_id);
            return;
        }

        case INST_INIT_STATIC_LOCAL: {
            fprintf(asm_file,
            ".data\n"
            ".balign %lu\n"
            ASM_SYMBOL_PREFIX ".%lu_%s_%s:\n"
            ".%s %lu\n"
            ".text\n",
            node.op_2.label_id, func_num, func_name, node.dst.identifier, asm_static_directive(node.op_2.label_id), node.op_1.immediate);
            return;
        }

        case INST_INIT_STATIC_Z_LOCAL: {
            fprintf(asm_file,
            ".bss\n"
            ".balign %lu\n"
            ASM_SYMBOL_PREFIX ".%lu_%s_%s:\n"
            ".zero %lu\n"
            ".text\n",
            node.op_2.label_id, func_num, func_name, node.dst.identifier, node.op_2.label_id);
            return;
        }

        case INST_SAVE_CALLER_REGS: {
            // fprintf(asm_file,
            // "pushq %%rdi\n"
            // "pushq %%rsi\n"
            // "pushq %%rdx\n"
            // "pushq %%rcx\n"
            // "pushq %%r8\n"
            // "pushq %%r9\n"
            // /* dont save rax as it is used for function returns */
            // "pushq %%r10\n"
            // "pushq %%r11\n"
            // /* ensure 16-byte alignment */
            // "subq $8, %%rsp\n");
            return;
        }

        case INST_LOAD_CALLER_REGS: {
            // fprintf(asm_file,
            // "addq $8, %%rsp\n"
            // "pop %%r11\n"
            // "pop %%r10\n"
            // "pop %%r9\n"
            // "pop %%r8\n"
            // "pop %%rcx\n"
            // "pop %%rdx\n"
            // "pop %%rsi\n"
            // "pop %%rdi\n");
            return;
        }

        case INST_SIGN_EXTEND_LQ: {
            str = strdup(codegen_val(node.op_1, INST_TYPE_LONGWORD));

            if (is_immediate(node.op_1.type))
                codegen_mov(node.op_1, node.dst, 8);
            else {
                if (node.dst.type > ___LVALUE_TYPE_START___)
                    fprintf(asm_file,
                    "movslq %s, %%rax\n"
                    "movq %%rax, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_QUADWORD));
                else
                    fprintf(asm_file,
                    "movslq %s, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_QUADWORD));
            }
            
            return;
        }

        case INST_ZERO_EXTEND_LQ: {
            str = strdup(codegen_val(node.op_1, INST_TYPE_LONGWORD));

            if (is_immediate(node.op_1.type))
                codegen_mov(node.op_1, node.dst, 8);
            else {
                if (node.dst.type > ___LVALUE_TYPE_START___)
                    fprintf(asm_file,
                    "movl %s, %%eax\n"
                    "movq %%rax, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_QUADWORD));
                else
                    fprintf(asm_file,
                    "movl %s, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_QUADWORD));
            }
            
            return;
        }

        case INST_SIGN_EXTEND_BW: {
            str = strdup(codegen_val(node.op_1, INST_TYPE_BYTEWORD));

            if (is_immediate(node.op_1.type))
                codegen_mov(node.op_1, node.dst, INST_TYPE_WORDWORD);
            else {
                if (node.dst.type > ___LVALUE_TYPE_START___)
                    fprintf(asm_file,
                    "movsbw %s, %%ax\n"
                    "movw %%ax, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_WORDWORD));
                else
                    fprintf(asm_file,
                    "movsbw %s, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_WORDWORD));
            }
            
            return;
        }
        
        case INST_ZERO_EXTEND_BW: {
            str = strdup(codegen_val(node.op_1, INST_TYPE_BYTEWORD));

            if (is_immediate(node.op_1.type))
                codegen_mov(node.op_1, node.dst, INST_TYPE_WORDWORD);
            else {
                if (node.dst.type > ___LVALUE_TYPE_START___)
                    fprintf(asm_file,
                    "movzbw %s, %%ax\n"
                    "movw %%ax, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_WORDWORD));
                else
                    fprintf(asm_file,
                    "movzbw %s, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_WORDWORD));
            }
            
            return;
        }

        case INST_SIGN_EXTEND_BL: {
            str = strdup(codegen_val(node.op_1, INST_TYPE_BYTEWORD));

            if (is_immediate(node.op_1.type))
                codegen_mov(node.op_1, node.dst, INST_TYPE_LONGWORD);
            else {
                if (node.dst.type > ___LVALUE_TYPE_START___)
                    fprintf(asm_file,
                    "movsbl %s, %%eax\n"
                    "movl %%eax, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_LONGWORD));
                else
                    fprintf(asm_file,
                    "movsbl %s, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_LONGWORD));
            }
            
            return;
        }
        
        case INST_ZERO_EXTEND_BL: {
            str = strdup(codegen_val(node.op_1, INST_TYPE_BYTEWORD));

            if (is_immediate(node.op_1.type))
                codegen_mov(node.op_1, node.dst, INST_TYPE_LONGWORD);
            else {
                if (node.dst.type > ___LVALUE_TYPE_START___)
                    fprintf(asm_file,
                    "movzbl %s, %%eax\n"
                    "movl %%eax, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_LONGWORD));
                else
                    fprintf(asm_file,
                    "movzbl %s, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_LONGWORD));
            }
            
            return;
        }

        case INST_SIGN_EXTEND_BQ: {
            str = strdup(codegen_val(node.op_1, INST_TYPE_BYTEWORD));

            if (is_immediate(node.op_1.type))
                codegen_mov(node.op_1, node.dst, INST_TYPE_QUADWORD);
            else {
                if (node.dst.type > ___LVALUE_TYPE_START___)
                    fprintf(asm_file,
                    "movsbq %s, %%rax\n"
                    "movq %%rax, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_QUADWORD));
                else
                    fprintf(asm_file,
                    "movsbq %s, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_QUADWORD));
            }
            
            return;
        }
        
        case INST_ZERO_EXTEND_BQ: {
            str = strdup(codegen_val(node.op_1, INST_TYPE_BYTEWORD));

            if (is_immediate(node.op_1.type))
                codegen_mov(node.op_1, node.dst, INST_TYPE_QUADWORD);
            else {
                if (node.dst.type > ___LVALUE_TYPE_START___)
                    fprintf(asm_file,
                    "movzbq %s, %%rax\n"
                    "movq %%rax, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_QUADWORD));
                else
                    fprintf(asm_file,
                    "movzbq %s, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_QUADWORD));
            }
            
            return;
        }

        case INST_SIGN_EXTEND_WL: {
            str = strdup(codegen_val(node.op_1, INST_TYPE_WORDWORD));

            if (is_immediate(node.op_1.type))
                codegen_mov(node.op_1, node.dst, INST_TYPE_LONGWORD);
            else {
                if (node.dst.type > ___LVALUE_TYPE_START___)
                    fprintf(asm_file,
                    "movswl %s, %%eax\n"
                    "movl %%eax, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_LONGWORD));
                else
                    fprintf(asm_file,
                    "movswl %s, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_LONGWORD));
            }
            
            return;
        }
        
        case INST_ZERO_EXTEND_WL: {
            str = strdup(codegen_val(node.op_1, INST_TYPE_WORDWORD));

            if (is_immediate(node.op_1.type))
                codegen_mov(node.op_1, node.dst, INST_TYPE_LONGWORD);
            else {
                if (node.dst.type > ___LVALUE_TYPE_START___)
                    fprintf(asm_file,
                    "movzwl %s, %%eax\n"
                    "movl %%eax, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_LONGWORD));
                else
                    fprintf(asm_file,
                    "movzwl %s, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_LONGWORD));
            }
            
            return;
        }

        case INST_SIGN_EXTEND_WQ: {
            str = strdup(codegen_val(node.op_1, INST_TYPE_WORDWORD));

            if (is_immediate(node.op_1.type))
                codegen_mov(node.op_1, node.dst, INST_TYPE_QUADWORD);
            else {
                if (node.dst.type > ___LVALUE_TYPE_START___)
                    fprintf(asm_file,
                    "movswq %s, %%rax\n"
                    "movq %%rax, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_QUADWORD));
                else
                    fprintf(asm_file,
                    "movswq %s, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_QUADWORD));
            }
            
            return;
        }
        
        case INST_ZERO_EXTEND_WQ: {
            str = strdup(codegen_val(node.op_1, INST_TYPE_WORDWORD));

            if (is_immediate(node.op_1.type))
                codegen_mov(node.op_1, node.dst, INST_TYPE_QUADWORD);
            else {
                if (node.dst.type > ___LVALUE_TYPE_START___)
                    fprintf(asm_file,
                    "movzwq %s, %%rax\n"
                    "movq %%rax, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_QUADWORD));
                else
                    fprintf(asm_file,
                    "movzwq %s, %s\n",
                    str, codegen_val(node.dst, INST_TYPE_QUADWORD));
            }
            
            return;
        }

        case INST_UDIV: {
            codegen_mov(node.op_1, (ir_operand_t){.type = REG_AX}, node.size);
            if (is_immediate(node.op_2.type)){
                str = strdup(codegen_val((ir_operand_t){.type = REG_AX}, node.size));

                const char* other_str = strdup(codegen_val((ir_operand_t){.type = REG_R10}, node.size));
                
                fprintf(asm_file,
                "cdq\n"
                "mov%1$c $%2$lu, %3$s\n"
                "div%1$c %3$s\n"
                "mov%1$c %4$s, %5$s\n",
                asm_inst_prefix(node.size), node.op_2.immediate, other_str, str, codegen_val(node.dst, node.size));
            }
            else {
                str = strdup(codegen_val(node.op_2, node.size));

                const char* other_str = strdup(codegen_val((ir_operand_t){.type = REG_AX}, node.size));

                fprintf(asm_file,
                "cdq\n"
                "div%1$c %2$s\n"
                "mov%1$c %3$s, %4$s\n",
                asm_inst_prefix(node.size), str, other_str, codegen_val(node.dst, node.size));
            }
            return;
        }

        case INST_UMOD: {
            codegen_mov(node.op_1, (ir_operand_t){.type = REG_AX}, node.size);
            char* edx_str = strdup(codegen_val((ir_operand_t){.type = REG_DX}, node.size));

            if (is_immediate(node.op_2.type)){
                str = strdup(codegen_val((ir_operand_t){.type = REG_R10}, node.size));

                fprintf(asm_file,
                "cdq\n"
                "mov%1$c $%2$lu, %3$s\n"
                "div%1$c %3$s\n"
                "mov%1$c %4$s, %5$s\n", 
                asm_inst_prefix(node.size), node.op_2.immediate, str, edx_str, codegen_val(node.dst, node.size));

                free(str);
            }
            else {
                char* other_str = strdup(codegen_val(node.op_2, node.size));

                fprintf(asm_file,
                "cdq\n"
                "div%1$c %2$s\n"
                "mov%1$c %3$s, %4$s\n",
                asm_inst_prefix(node.size), other_str, edx_str, codegen_val(node.dst, node.size));

                free(other_str);
            }
            free(edx_str);
            return;
        }
    }
}

static const char* REG_NAMES[][4] = {
    /* rax */
    { "%al", "%ax", "%eax", "%rax" },
    /* rbx */
    { "%bl", "%bx", "%ebx", "%rbx" },
    /* rcx */
    { "%cl", "%cx", "%ecx", "%rcx" },
    /* rdx */
    { "%dl", "%dx", "%edx", "%rdx" },
    /* rsi */
    { "%sil", "%si", "%esi", "%rsi" },
    /* rdi */
    { "%dil", "%di", "%edi", "%rdi" },
    /* rbp */
    { "%bpl", "%db", "%ebp", "%rbp" },
    /* rsp */
    { "%spl", "%sp", "%esp", "%rsp" },
    /* r8 */
    { "%r8b", "%r8w", "%r8d", "%r8" },
    /* r9 */
    { "%r9b", "%r9w", "%r9d", "%r9" },
    /* r10 */
    { "%r10b", "%r10w", "%r10d", "%r10" },
    /* r11 */
    { "%r11b", "%r11w", "%r11d", "%r11" },
    /* r12 */
    { "%r12b", "%r12w", "%r12d", "%r12" },
    /* r13 */
    { "%r13b", "%r13w", "%r13d", "%r13" },
    /* r14 */
    { "%r14b", "%r14w", "%r14d", "%r14" },
    /* r15 */
    { "%r15b", "%r15w", "%r15d", "%r15" },
};