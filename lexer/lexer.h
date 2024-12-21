#ifndef LEXER_H
#define LEXER_H

#define LEXER_TYPES
#include "../globals.h"

#include "../lib/vector/C_vector.h"

#include <inttypes.h>

token_t* lex(const char* fpath);

void token_destr(token_t* token);

#endif