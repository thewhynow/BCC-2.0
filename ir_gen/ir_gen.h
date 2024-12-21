#ifndef IR_GEN_H
#define IR_GEN_H

#include "../parser/parser.h"
#define PARSER_TYPES
#define IR_GEN_TYPES
#include "../globals.h"

#include "../lib/vector/C_vector.h"

ir_func_t* ir_gen(node_t* node);

#endif