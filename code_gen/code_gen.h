#ifndef CODE_GEN_H
#define CODE_GEN_H

#include "../parser/parser.h"
#define IR_GEN_TYPES
#include "../globals.h"

void codegen(ir_func_t* nodes, const char* asm_file);

#endif