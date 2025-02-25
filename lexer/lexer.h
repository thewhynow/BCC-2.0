#ifndef LEXER_H
#define LEXER_H

#define LEXER_TYPES
#include "../globals.h"

#include "../lib/vector/C_vector.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

token_t* lex(const char* fpath);

void token_destr(token_t* token);

#endif