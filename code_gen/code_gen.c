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

extern char** func_names;

void codegen(ir_node_t* nodes, const char* asm_file_path){
    asm_file = fopen(asm_file_path, "w");

    if (!asm_file)
        fprintf(stderr, "failed to output assembly: %s\n", asm_file_path);
    
    fprintf(asm_file, ".text\n");

    for (size_t i = 0; i < get_count_array(nodes); ++i)
        codegen_node(nodes[i]);

    extern ir_node_t* read_only_initializers;
    extern ir_node_t* static_initializers;
    extern ir_node_t* zero_initializers;


    fprintf(asm_file, ".data\n");
    for (size_t i = 0; i < get_count_array(static_initializers); ++i)
        codegen_node(static_initializers[i]);

    #ifdef __APPLE__
        #define STRING_SECTION ".cstring"
    #endif
    #ifdef __linux__
        #define STRING_SECTION ".section .rodata"
    #endif
    
    fprintf(asm_file, STRING_SECTION "\n");
    for (size_t i = 0; i < get_count_array(read_only_initializers); ++i)
        codegen_node(read_only_initializers[i]);

    fprintf(asm_file, ".bss\n");
    for (size_t i = 0; i < get_count_array(zero_initializers); ++i)
        codegen_node(zero_initializers[i]);

    #ifdef __linux__
        fprintf(asm_file, ".section .note.GNU-stack, \"\",@progbits\n");
    #endif

    fclose(asm_file);

    free_array(zero_initializers, NULL);
    free_array(static_initializers, NULL);
    free_array(read_only_initializers, NULL);
    free_array(func_names, NULL);

    extern scope_t* global_scope;
    void symbol_dstr(void* _symbol);
    free_array(global_scope->top_scope, symbol_dstr);
    free(global_scope);
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
            snprintf(str_buff, 4096, ASM_SYMBOL_PREFIX "%s(%%rip)", val.identifier);
            return str_buff;
        case STATIC_MEM_LOCAL:
            snprintf(str_buff, 4096, ASM_SYMBOL_PREFIX ".%lu_%s_%s(%%rip)", func_num, func_name, val.identifier);
            return str_buff;
        case MEM_ADDRESS:
            return "(%r11)";
        case STRING:
            snprintf(str_buff, 4096, ".string_%lu(%%rip)", val.label_id);
            return str_buff;
        case STRING_ADDRESS:
            snprintf(str_buff, 4096, ".string_%lu", val.label_id);
            return str_buff;
        case FUNCTION:{
            #ifdef __APPLE__
                #if WARN_STDLIB_CALL
                    fprintf(stderr, "WARNING: Creating a function pointer from a stdlib-function may not work on macOS\n");
                #endif 
               
                snprintf(str_buff, 4096, "man 3 %s > /dev/null 2>&1", val.identifier);
               
                if (system(str_buff))
                    snprintf(str_buff, 4096, "_%s(%%rip)", val.identifier);
                else
                    snprintf(str_buff, 4096, "_%s@GOTPCREL(%%rip)", val.identifier);
            #else
                snprintf(str_buff, 4096, ASM_SYMBOL_PREFIX "%s(%%rip)", val.identifier);
            #endif

            return str_buff;
        }
        case STATIC_MEM_OFFSET: {
            snprintf(str_buff, 4096, ASM_SYMBOL_PREFIX "%s%+ld(%%rip)", val.identifier, val.secondary_offset);
            return str_buff;
        }
        case STATIC_MEM_LOCAL_OFFSET:{
            snprintf(str_buff, 4096, ASM_SYMBOL_PREFIX ".%lu_%s_%s%+ld(%%rip)", func_num, func_name, val.identifier, val.secondary_offset);
            return str_buff;
        }
        case MEM_ADDRESS_OFFSET: {
            snprintf(str_buff, 4096, "%ld(%%r11)", val.secondary_offset);
            return str_buff;
        }
        default:
            return NULL;
    }
}

const char* codegen_static_val(ir_operand_t val, size_t size){

    static char buff[4096];

    switch(val.type){
        case IMM_U32:
        case IMM_U64:
        case IMM_U16:
        case IMM_U8:
            snprintf(buff, 4096, "%lu", val.immediate);
            return buff;
        case STRING:
            snprintf(buff, 4096,  ".string_%lu", val.label_id);
            return buff;
        case STRING_ADDRESS:
            snprintf(buff, 4096,  ".string_%lu", val.label_id);
            return buff;
        default:
            return NULL;
    }
}

ir_operand_t get_val_offset(ir_operand_t val, long offset);

void codegen_mov(ir_operand_t src, ir_operand_t dst, size_t size){
    static char alt_val_buff[4096];

    if (size == 1 || size == 2 || size == 4 || size == 8){
        strcpy(alt_val_buff, codegen_val(src, size));

        char* r10_str = strdup(codegen_val((ir_operand_t){.type = REG_R10}, size));
        
        if (src.type > ___LVALUE_TYPE_START___ && dst.type > ___LVALUE_TYPE_START___){
            fprintf(asm_file,
            "mov%1$c %2$s, %3$s\n"
            "mov%1$c %3$s, %4$s\n",
            asm_inst_prefix(size), alt_val_buff, r10_str, codegen_val(dst, size));
        }
        else
            fprintf(asm_file, 
                "%s %s, %s\n", 
                generate_asm_inst("mov", size), alt_val_buff, codegen_val(dst, size));

        free(r10_str);
    }
    else {
        if (dst.type  < ___IMMEDIATE_TYPE_START___)
            size = (5 <= size && size <= 7) ? 8 : (size == 3) ? 4 : size;

        if (((5 <= size && size <= 7) || size == 3) && src.type < ___IMMEDIATE_TYPE_START___){
            char* dst_str;
            char* src_str;
            size_t offset = 0;

            if (size >= 5){
                dst_str = strdup(codegen_val(dst, 4));
                src_str = strdup(codegen_val(src, 8));
                fprintf(asm_file,
                "movl %s, %s\n"
                "shrq $32, %s\n",
                codegen_val(src, 4), dst_str, src_str);

                free(dst_str);
                free(src_str);

                size -= 4;

                offset = 4;
            }

            if (size == 3){
                src_str = strdup(codegen_val(src, 8));
                char* src_str_short = strdup(codegen_val(src, 2));
                char* src_str_byte = strdup(codegen_val(src, 1));
                dst_str = strdup(codegen_val(get_val_offset(dst, offset), 2));

                fprintf(asm_file,
                "movw %s, %s\n"
                "shrq $16, %s\n"
                "movb %s, %s\n",
                src_str_short, dst_str, src_str, src_str_byte, codegen_val(get_val_offset(dst, offset + 2), 1));

                free(src_str);
                free(dst_str);
                free(src_str_short);
                free(src_str_byte);

                size -= 3;
            } else
            if (size == 2){
                src_str = strdup(codegen_val(src, 2));                    
                
                fprintf(asm_file,
                "movw %s, %s\n",
                src_str, codegen_val(get_val_offset(dst, offset), 2));

                free(src_str);

                size -= 2;
            }
            else {
                src_str = strdup(codegen_val(src, 1));
                fprintf(asm_file,
                "movb %s, %s\n",
                src_str, codegen_val(get_val_offset(dst, offset), 1));
                free(src_str);

                --size;
            }
        }

        size_t begin_size = size;

        char* src_str;

        for (; size >= 8; size -= 8){
            src_str = strdup(codegen_val(get_val_offset(src, begin_size - size), 8));
            char* dst_str = (char*)codegen_val(get_val_offset(dst, begin_size - size), 8);
            fprintf(asm_file,
            "movq %s, %%r10\n"
            "movq %%r10, %s\n",
            src_str, dst_str);
            free(src_str);
        }

        if (size >= 4){
            src_str = strdup(codegen_val(get_val_offset(src, begin_size - size), 4));
            fprintf(asm_file,
            "movl %s, %%r10d\n"
            "movl %%r10d, %s\n",
            src_str, codegen_val(get_val_offset(dst, begin_size - size), 4));
            free(src_str);
            size -= 4;
        }

        if (size >= 2){
            src_str = strdup(codegen_val(get_val_offset(src, begin_size - size), 2));
            fprintf(asm_file,
            "movw %s, %%r10w\n"
            "movw %%r10w, %s\n",
            src_str, codegen_val(get_val_offset(dst, begin_size - size), 2));
            free(src_str);
            size -= 2;
        }

        if (size){
            src_str = strdup(codegen_val(get_val_offset(src, begin_size - size), 1));
            fprintf(asm_file,
            "movb %s, %%r10b\n"
            "movb %%r10b, %s\n",
            src_str, codegen_val(get_val_offset(dst, begin_size - size), 1));
            free(src_str);
        }
    }
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
        if (b.type > ___LVALUE_TYPE_START___ && a.type > ___LVALUE_TYPE_START___){
            codegen_mov(a, (ir_operand_t){.type = REG_R10}, size);

            char* str = strdup(codegen_val((ir_operand_t){.type = REG_R10}, size));

            fprintf(asm_file,
            "%s%c %s, %s\n",
            inst, inst_suffix, codegen_val(b, size), str);

            free(str);
        }
        /* this is only used for cmp instructions, so the order of the operands matter */
        else {
            static char b_val_str[4096];

            strncpy(b_val_str, codegen_val(b, size), 4096);

            if (b.type > ___LVALUE_TYPE_START___){
                codegen_mov(a, (ir_operand_t){.type = REG_R10}, size);

                char* r10_str = strdup(codegen_val((ir_operand_t){.type = REG_R10}, size));

                fprintf(asm_file,
                "%s%c %s, %s\n",
                inst, inst_suffix, codegen_val(b, size), r10_str);

                free(r10_str);
            }
            else {
                fprintf(asm_file,
                "pushq %%r12\n");

                codegen_mov(b, (ir_operand_t){.type = REG_R12}, size);

                b.type = REG_R12;

                codegen_mov(a, (ir_operand_t){.type = REG_R10}, size);

                char* r10_str = strdup(codegen_val((ir_operand_t){.type = REG_R10}, size));

                fprintf(asm_file,
                "%s%c %s, %s\n",
                inst, inst_suffix, codegen_val(b, size), r10_str);

                fprintf(asm_file,
                "popq %%r12\n");

                free(r10_str);
            }
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
            
            free(str);
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
            if (node.op_2.type == REG_AX){
                codegen_mov(node.op_2, (ir_operand_t){.type = REG_R10}, node.size);
                node.op_2.type = REG_R10;
            }

            codegen_mov(node.op_1, (ir_operand_t){.type = REG_AX}, node.size);

            const char* extend_str = (
                node.size == 4 ? "cdq" :
                node.size == 8 ? "cqo" :
                node.size == 2 ? "cwd" :
                node.size == 1 ? "cwd" :
                NULL
            );

            if (node.size == 1){
                fprintf(asm_file,
                "movsbw %%al, %%ax\n");
            }

            if (is_immediate(node.op_2.type)){
                str = strdup(codegen_val((ir_operand_t){.type = REG_AX}, node.size));

                char* other_str = strdup(codegen_val((ir_operand_t){.type = REG_R10}, node.size));
                
                fprintf(asm_file,
                "%6$s\n"
                "mov%1$c $%2$lu, %3$s\n"
                "idiv%1$c %3$s\n"
                "mov%1$c %4$s, %5$s\n",
                asm_inst_prefix(node.size), node.op_2.immediate, other_str, str, codegen_val(node.dst, node.size), extend_str);

                free(other_str);
            }
            else {
                str = strdup(codegen_val(node.op_2, node.size));

                char* other_str = strdup(codegen_val((ir_operand_t){.type = REG_AX}, node.size));

                fprintf(asm_file,
                "%5$s\n"
                "idiv%1$c %2$s\n"
                "mov%1$c %3$s, %4$s\n",
                asm_inst_prefix(node.size), str, other_str, codegen_val(node.dst, node.size), extend_str);

                free(other_str);
            }

            free(str);
            return;
        }

        case INST_MOD: {
            codegen_mov(node.op_1, (ir_operand_t){.type = REG_AX}, node.size);
            char* edx_str = strdup(codegen_val((ir_operand_t){.type = REG_DX}, node.size));

            const char* extend_str = (
                node.size == 4 ? "cdq" :
                node.size == 8 ? "cqo" :
                node.size == 2 ? "cwd" :
                node.size == 1 ? "cwd" :
                NULL
            );

            if (node.size == 1){
                fprintf(asm_file,
                "movsbw %%al, %%ax\n");
            }

            if (is_immediate(node.op_2.type)){
                str = strdup(codegen_val((ir_operand_t){.type = REG_R10}, node.size));

                fprintf(asm_file,
                "%6$s\n"
                "mov%1$c $%2$lu, %3$s\n"
                "idiv%1$c %3$s\n"
                "mov%1$c %4$s, %5$s\n", 
                asm_inst_prefix(node.size), node.op_2.immediate, str, edx_str, codegen_val(node.dst, node.size), extend_str);

                free(str);
            }
            else {
                char* other_str = strdup(codegen_val(node.op_2, node.size));

                fprintf(asm_file,
                "%5$s\n"
                "idiv%1$c %2$s\n"
                "mov%1$c %3$s, %4$s\n",
                asm_inst_prefix(node.size), other_str, edx_str, codegen_val(node.dst, node.size), extend_str);

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
            if (node.dst.type == FUNCTION){
                fprintf(asm_file,
                "call "ASM_SYMBOL_PREFIX"%s\n",
                node.dst.identifier);
            }
            else {
                #ifdef __APPLE__
                    fprintf(asm_file,
                    "call *%s\n",
                    codegen_val(node.dst, 8));
                #else
                    fprintf(asm_file,
                    "call *%s\n",
                    codegen_val(node.dst, 8));
                #endif
            }
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
            ".balign %lu\n"
            ASM_SYMBOL_PREFIX "%s:\n"
            ".%s %lu\n",
            16LU, node.dst.identifier, asm_static_directive(node.op_2.label_id), node.op_1.immediate);
            return;
        }

        case INST_INIT_STATIC_Z: {
            fprintf(asm_file,
            ".balign %lu\n"
            ASM_SYMBOL_PREFIX "%s:\n"
            ".zero %lu\n",
            16LU, node.dst.identifier, node.op_2.label_id);
            return;
        }

        case INST_INIT_STATIC_PUBLIC: {
            fprintf(asm_file,
            ".globl " ASM_SYMBOL_PREFIX "%s\n"
            ".balign %lu\n"
            ASM_SYMBOL_PREFIX "%s:\n"
            ".%s %lu\n",
            node.dst.identifier, 16LU, node.dst.identifier, asm_static_directive(node.size), node.op_1.immediate);
            return;
        }

        case INST_INIT_STATIC_Z_PUBLIC: {
            fprintf(asm_file,
            ".globl " ASM_SYMBOL_PREFIX "%s\n"
            ".balign %lu\n"
            ASM_SYMBOL_PREFIX "%s:\n"
            ".zero %lu\n",
            node.dst.identifier, 16LU, node.dst.identifier, node.op_2.label_id);
            return;
        }

        case INST_INIT_STATIC_LOCAL: {
            fprintf(asm_file,
            ".balign %lu\n"
            ASM_SYMBOL_PREFIX ".%lu_%s_%s:\n"
            ".%s %lu\n"
            ".text\n",
            16LU, node.size, func_names[node.size], node.dst.identifier, asm_static_directive(node.op_2.label_id), node.op_1.immediate);
            return;
        }

        case INST_INIT_STATIC_Z_LOCAL: {
            fprintf(asm_file,
            ".balign %lu\n"
            ASM_SYMBOL_PREFIX ".%lu_%s_%s:\n"
            ".zero %lu\n",
            16LU, node.size, func_names[node.size], node.dst.identifier, node.op_2.label_id);
            return;
        }

        case INST_SAVE_CALLER_REGS: {
            fprintf(asm_file,
            "pushq %%rdi\n"
            "pushq %%rsi\n"
            "pushq %%rdx\n"
            "pushq %%rcx\n"
            "pushq %%r8\n"
            "pushq %%r9\n"
            /* dont save rax as it is used for function returns */
            "pushq %%r10\n"
            "pushq %%r11\n");
            return;
        }

        case INST_LOAD_CALLER_REGS: {
            fprintf(asm_file,
            "pop %%r11\n"
            "pop %%r10\n"
            "pop %%r9\n"
            "pop %%r8\n"
            "pop %%rcx\n"
            "pop %%rdx\n"
            "pop %%rsi\n"
            "pop %%rdi\n");
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

            free(str);
            
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

            free(str);
            
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

            free(str);
            
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

            free(str);
            
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

            free(str);
            
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
            free(str);
            
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

            free(str);
            
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

            free(str);
            
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

            free(str);
            
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

            free(str);
            
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

            free(str);
            
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
            
            free(str);

            return;
        }

        case INST_UDIV: {
            codegen_mov(node.op_1, (ir_operand_t){.type = REG_AX}, node.size);

            fprintf(asm_file,
            "movq $0, %%rdx\n");

            if (is_immediate(node.op_2.type)){
                str = strdup(codegen_val((ir_operand_t){.type = REG_AX}, node.size));

                char* other_str = strdup(codegen_val((ir_operand_t){.type = REG_R10}, node.size));
                
                fprintf(asm_file,
                "mov%1$c $%2$lu, %3$s\n"
                "div%1$c %3$s\n"
                "mov%1$c %4$s, %5$s\n",
                asm_inst_prefix(node.size), node.op_2.immediate, other_str, str, codegen_val(node.dst, node.size));

                free(other_str);
                free(str);
            }
            else {
                str = strdup(codegen_val(node.op_2, node.size));

                char* other_str = strdup(codegen_val((ir_operand_t){.type = REG_AX}, node.size));

                fprintf(asm_file,
                "div%1$c %2$s\n"
                "mov%1$c %3$s, %4$s\n",
                asm_inst_prefix(node.size), str, other_str, codegen_val(node.dst, node.size));

                free(other_str);
                free(str);
            }
            return;
        }

        case INST_UMOD: {
            codegen_mov(node.op_1, (ir_operand_t){.type = REG_AX}, node.size);
            char* edx_str = strdup(codegen_val((ir_operand_t){.type = REG_DX}, node.size));

            if (is_immediate(node.op_2.type)){
                str = strdup(codegen_val((ir_operand_t){.type = REG_R10}, node.size));

                fprintf(asm_file,
                "mov%1$c $%2$lu, %3$s\n"
                "div%1$c %3$s\n"
                "mov%1$c %4$s, %5$s\n", 
                asm_inst_prefix(node.size), node.op_2.immediate, str, edx_str, codegen_val(node.dst, node.size));

                free(str);
            }
            else {
                char* other_str = strdup(codegen_val(node.op_2, node.size));

                fprintf(asm_file,
                "div%1$c %2$s\n"
                "mov%1$c %3$s, %4$s\n",
                asm_inst_prefix(node.size), other_str, edx_str, codegen_val(node.dst, node.size));

                free(other_str);
            }
            free(edx_str);
            return;
        }

        case INST_GET_ADDRESS: {
            str = strdup(codegen_val(node.op_1, node.size));
            
            if (node.dst.type > ___LVALUE_TYPE_START___){
                fprintf(asm_file,
                "leaq %s, %%rax\n"
                "movq %%rax, %s\n",
                str, codegen_val(node.dst, node.size));
            }
            else
                fprintf(asm_file,
                "leaq %s, %s\n",
                str, codegen_val(node.dst, node.size));

            free(str);
            return;
        }

        case INST_SUB_POINTER:
        case INST_ADD_POINTER: {
            fprintf(asm_file,
            "pushq %%rdx\n");
            
            codegen_mov(node.op_1, (ir_operand_t){.type = REG_AX}, 8);
            codegen_mov(node.op_2, (ir_operand_t){.type = REG_DX}, 8);

            if (node.instruction == INST_SUB_POINTER)
                fprintf(asm_file,
                    "negq %%rdx\n");

            if (node.size == 1 || node.size == 2 || node.size == 4 || node.size == 8)
                fprintf(asm_file,
                "leaq (%%rax, %%rdx, %lu), %%rax\n"
                "movq %%rax, %s\n",
                node.size, codegen_val(node.dst, 8));
            else {
                codegen_node((ir_node_t){
                    .instruction = INST_MUL,
                    .dst = (ir_operand_t){.type = REG_R10},
                    .op_1 = (ir_operand_t){.type = IMM_U64, .immediate = node.size},
                    .op_2 = (ir_operand_t){.type = REG_DX},
                    .size = 8
                });

                fprintf(asm_file,
                "leaq (%%rax, %%r10, 1), %%rax\n"
                "movq %%rax, %s\n",
                codegen_val(node.dst, 8));
            }

            fprintf(asm_file,
            "popq %%rdx\n");

            return;
        }

        case INST_STATIC_ARRAY:
        case INST_STATIC_ARRAY_PUBLIC: {
            if (node.instruction == INST_STATIC_ARRAY_PUBLIC)
                fprintf(asm_file,
                ".globl " ASM_SYMBOL_PREFIX "%s\n",
                node.dst.identifier);

            fprintf(asm_file,
            ASM_SYMBOL_PREFIX "%s:\n"
            ".balign 16\n",
            node.dst.identifier);

            return;
        }

        case INST_STATIC_ELEM: {
            fprintf( asm_file,
            ".%s %s\n",
            asm_static_directive(node.op_2.label_id), codegen_static_val(node.op_1, node.size));

            return;
        }

        case INST_STATIC_ARRAY_LOCAL: {
            fprintf(asm_file,
            ASM_SYMBOL_PREFIX ".%lu_%s_%s:\n",
            node.size, func_names[node.size], node.dst.identifier);

            if (node.op_1.label_id < 16)
                fprintf(asm_file, 
                ".balign %lu\n", 
                node.op_2.label_id);
            else
                fprintf(asm_file, 
                ".balign 16\n");

            return;
        }

        case INST_STATIC_STRING: {
            fprintf(asm_file,
            ".string_%lu:\n"
            ".asciz \"",
            node.op_1.label_id);

            for (char* c = node.dst.identifier; *c; ++c){

                switch(*c){
                        case '\0':
                            fprintf(asm_file, "\\0");
                            break;
                        case '\\':
                            fprintf(asm_file, "\\\\");
                            break;
                        case '\n':
                            fprintf(asm_file, "\\n");
                            break;
                        case '\'':
                            fprintf(asm_file, "\\'");
                            break;
                        case '\"':
                            fprintf(asm_file, "\\\"");
                            break;
                        case '\?': {
                            #ifdef __APPLE__
                                fprintf(asm_file, "?");
                            #else
                                fprintf(asm_file, "\\?");    
                            #endif
                            break;
                        }
                        case '\a':
                            fprintf(asm_file, "\\a");
                            break;
                        case '\b':
                            fprintf(asm_file, "\\b");
                            break;
                        case '\f':
                            fprintf(asm_file, "\\f");
                            break;
                        case '\r':
                            fprintf(asm_file, "\\r");
                            break;
                        case '\t':
                            fprintf(asm_file, "\\t");
                            break;
                        case '\v':
                            fprintf(asm_file, "\\v");
                            break;

                        default:
                            fprintf(asm_file, "%c", *c);
                    }
            }

            fprintf(asm_file,
            "\"\n");

            return;
        }

        case INST_STATIC_STRING_P: {
            fprintf(asm_file,
            ASM_SYMBOL_PREFIX "%s:\n"
            ".balign 8\n"
            ".quad .string_%lu\n",
            node.dst.identifier, node.op_1.label_id);

            return;
        }
        
        case INST_STATIC_STRING_P_PUBLIC: {
            fprintf(asm_file,
            ".globl %1$s\n"
            ASM_SYMBOL_PREFIX "%1$s:\n"
            ".balign 8\n"
            ".quad .string_%2$lu\n"
            ,
            node.dst.identifier, node.op_1.label_id);

            return;
        }
        
        case INST_STATIC_STRING_P_LOCAL: {
            fprintf(asm_file,
            ".globl %1$s\n"
            ".%2$lu_%3$s_%1$s:\n"
            ".balign 8\n"
            ".quad .string_%4$lu\n"
            ,
            node.dst.identifier, node.size, func_names[node.size], node.op_1.label_id);

            return;
        }

        case INST_LOAD_ADDRESS: {
            fprintf(asm_file,
            "movq %1$s, %%r11\n",
            codegen_val(node.op_1, 8));

            codegen_mov((ir_operand_t){.type = MEM_ADDRESS}, node.dst, 8);

            return;
        }

        case INST_STORE_ADDRESS: {
            fprintf(asm_file,
            "movq %s, %%r11\n",
            codegen_val(node.dst, 8));

            if (node.op_1.type > ___LVALUE_TYPE_START___){
                char* r10_str = strdup(codegen_val((ir_operand_t){.type = REG_R10}, node.size));

                fprintf(asm_file,
                "mov%1$c %2$s, %%r10\n"
                "mov%1$c %3$s, (%%r11)\n",
                asm_inst_prefix(node.size), codegen_val(node.op_1, node.size), r10_str);

                free(r10_str);
            }
            else {
                fprintf(asm_file,
                "mov%1$c %2$s, (%%r11)\n",
                asm_inst_prefix(node.size), codegen_val(node.op_1, node.size));
            }

            return;
        }

        case INST_COPY_FROM_OFFSET: {
            if (node.op_1.type == STK_OFFSET){
                if (node.dst.type > ___LVALUE_TYPE_START___){
                    char* r11_str = strdup(codegen_val((ir_operand_t){.type = REG_R11}, node.size));
                    fprintf(asm_file,
                    "mov%1$c %2$ld(%%rbp), %3$s\n"
                    "mov%1$c %3$s, %4$s\n",
                    asm_inst_prefix(node.size), (signed long)node.op_2.immediate + node.op_1.offset, r11_str, codegen_val(node.dst, node.size));
                    free(r11_str);
                }
                else
                    fprintf(asm_file,
                    "mov%1$c %2$ld(%%rbp), %3$s\n",
                    asm_inst_prefix(node.size), (signed long)node.op_2.immediate + node.op_1.offset, codegen_val(node.dst, node.size));
            }
            else {
                fprintf(asm_file,
                "lea %s, %%r11\n",
                codegen_val(node.op_1, node.size));

                if (node.dst.type > ___LVALUE_TYPE_START___){
                    char* r11_str = strdup(codegen_val((ir_operand_t){.type = REG_R11}, node.size));
                    fprintf(asm_file,
                    "mov%1$c %2$lu(%%r11), %3$s\n"
                    "mov%1$c %3$s, %4$s\n",
                    asm_inst_prefix(node.size), node.op_2.immediate, r11_str, codegen_val(node.dst, node.size));
                    free(r11_str);
                }
                else
                    fprintf(asm_file,
                    "mov%1$c %2$lu(%%r11), %3$s\n",
                    asm_inst_prefix(node.size), node.op_2.immediate, codegen_val(node.dst, node.size));
            }
            return;
        }

        case INST_COPY_TO_OFFSET: {
            fprintf(asm_file,
            "pushq %%r12\n"
            "lea %s, %%r12\n",
            codegen_val(node.dst, node.size));

            if (node.op_1.type > ___LVALUE_TYPE_START___){
                char* r10_str = strdup(codegen_val((ir_operand_t){.type = REG_R10}, node.size));
                fprintf(asm_file,
                "mov%1$c %2$s, %3$s\n"
                "mov%1$c %3$s, %4$lu(%%r12)\n",
                asm_inst_prefix(node.size), codegen_val(node.op_1, node.size), r10_str, node.op_2.immediate);

                free(r10_str);
            }
            else
                fprintf(asm_file,
                "mov%1$c %2$s, %3$lu(%%r12)\n",
                asm_inst_prefix(node.size), codegen_val(node.op_1, node.size), node.op_2.immediate);

            fprintf(asm_file,
            "popq %%r12\n");

            return;
        }

        case INST_ZERO: {
            fprintf(asm_file,
            ".zero %lu\n",
            node.op_1.label_id
            );

            return;
        }

        default: {
            fprintf(stderr, "Unknown instruction: %d\n", node.instruction);
            exit(-1);
        }

    }
}

static const char* REG_NAMES[16][4] = {
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