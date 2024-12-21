#ifndef PARSER_H
#define PARSER_H

#define LEXER_TYPES
#define PARSER_TYPES
#include "../globals.h"

#include "../lib/vector/C_vector.h"

node_t* parse(token_t* tokens, const char* fpath);

#endif