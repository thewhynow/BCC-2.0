#include "lexer.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

/* return TOK_NULL (0) on false */
token_type_t is_keyword(const char* str, size_t* increment_counter);

token_t* lex(const char* fpath){
    FILE* c_file = fopen(fpath, "r");

    if (!c_file){
        fprintf(stderr, "%s: Unable to open file", fpath);
        exit(-1);
    }

    char* line_str = malloc(sizeof(char) * 4096);

    if (!line_str){
        fprintf(stderr, "%s: Ran out of memory while compiling", fpath);
        exit(-1);
    }

    size_t line_count = 1;

    token_t* tok_vec = alloc_array(sizeof(token_t), 1);

    token_t token;

    while (!feof(c_file)){
        memset(line_str, 0, 4096);
        if (!fread(line_str, 1, 4096, c_file)){
            fprintf(stderr, "%s.%lu: Error reading file\n", fpath, line_count);
            exit(-1);
        }

        size_t line_str_len = strlen(line_str);
        
        for (size_t i = 0; i < line_str_len; ++i){
            char c = line_str[i];
            switch(c){
                case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': {
                    unsigned int num = 0;

                    while ('0' <= c && c <= '9'){
                        num *= 10;
                        num += c - 48;
                        c = line_str[++i];
                    }

                    --i;

                    token = (token_t){
                        .type = TOK_INTEGER,
                        .uint_literal = num,
                        .line = line_count
                    };

                    pback_array(&tok_vec, &token);

                    break;
                }

                case '(':{
                    token = (token_t){
                        .type = TOK_OPEN_PAREN,
                        .line = line_count
                    };
                    pback_array(&tok_vec, &token);

                    break;
                }
                
                case ')':{
                    token = (token_t){
                        .type = TOK_CLOSE_PAREN,
                        .line = line_count
                    };
                    pback_array(&tok_vec, &token);
                    
                    break;
                }

                case '{':{
                    token = (token_t){
                        .type = TOK_OPEN_BRACE,
                        .line = line_count
                    };
                    pback_array(&tok_vec, &token);

                    break;
                }

                case '}':{
                    token = (token_t){
                        .type = TOK_CLOSE_BRACE,
                        .line = line_count
                    };
                    pback_array(&tok_vec, &token);

                    break;
                }

                case ';':{
                    token = (token_t){
                        .type = TOK_SEMICOLON,
                        .line = line_count
                    };
                    pback_array(&tok_vec, &token);

                    break;
                }

                case '_': case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z': {
                    
                    token_type_t keyw_type = is_keyword(line_str + i, &i);
                    
                    if (keyw_type){
                        token = (token_t){
                            .type = keyw_type,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                        
                        break;
                    }
                    else {
                        size_t len = 0;
                        do {
                            ++len;
                        } while (is_identifier_char(line_str[i + len]));

                        char* identifier = malloc(sizeof(char) * len + 1);
                        if (identifier){
                            strncpy(identifier, line_str + i, len);
                            identifier[len] = '\0';
                        }
                        else {
                            fprintf(stderr, "%s: Ran out of memory while compiling\n", fpath);
                            exit(-1);
                        }

                        token = (token_t){
                            .type = TOK_IDENTIFIER,
                            .identifier = identifier,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);

                        i += len - 1;
                        break;
                    }

                }

                case '-':{
                    token = (token_t){
                        .type = TOK_SUB,
                        .line = line_count
                    };
                    pback_array(&tok_vec, &token);
                    break;
                }

                case '!':{
                    token = (token_t){
                        .type = TOK_LOGICAL_NOT,
                        .line = line_count
                    };
                    pback_array(&tok_vec, &token);
                    break;
                }

                case '~':{
                    token = (token_t){
                        .type = TOK_BITWISE_NOT,
                        .line = line_count
                    };
                    pback_array(&tok_vec, &token);
                    break;
                }
                
                case '+':{
                    token = (token_t){
                        .type = TOK_ADD,
                        .line = line_count
                    };
                    pback_array(&tok_vec, &token);
                    break;
                }

                case '&':{
                    if ((i < (line_str_len - 1)) && (line_str[i + 1] == '&')){
                        token = (token_t){
                            .type = TOK_LOGICAL_AND,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                        ++i;
                    }
                    else {
                        token = (token_t){
                            .type = TOK_BITWISE_AND,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                    }
                    break;
                }

                case '|':{
                    if ((i < (line_str_len - 1)) && (line_str[i + 1] == '|')){
                        token = (token_t){
                            .type = TOK_LOGICAL_OR,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                        ++i;
                    }
                    else {
                        token = (token_t){
                            .type = TOK_BITWISE_OR,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                    }
                    break;
                }
                
                case '^':{
                    token = (token_t){
                        .type = TOK_BITWISE_XOR,
                        .line = line_count
                    };
                    pback_array(&tok_vec, &token);
                    break;
                }

                case '<':{
                    if ((i < (line_str_len - 1)) && (line_str[i + 1] == '<')){
                        token = (token_t){
                            .type = TOK_SHIFT_LEFT,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                        ++i;
                    } else
                    if ((i < (line_str_len - 1)) && (line_str[i + 1] == '=')) {
                        token = (token_t){
                            .type = TOK_LESS_THAN_OR_EQUAL,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                        ++i;
                    }
                    else {
                        token = (token_t){
                            .type = TOK_LESS_THAN,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                    }
                    break;
                }

                case '>':{
                    if ((i < (line_str_len - 1)) && (line_str[i + 1] == '>')){
                        token = (token_t){
                            .type = TOK_SHIFT_RIGHT,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                        ++i;
                    } else
                    if ((i < (line_str_len - 1)) && (line_str[i + 1] == '=')) {
                        token = (token_t){
                            .type = TOK_GREATER_THAN_OR_EQUAL,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                        ++i;
                    }
                    else {
                        token = (token_t){
                            .type = TOK_GREATER_THAN,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                    }
                    break;
                }

                case '\t': case ' ':{
                    break;
                }

                case '\n':{
                    ++line_count;
                    break;
                }

                case '*':{
                    token = (token_t){
                        .type = TOK_MUL,
                        .line = line_count
                    };
                    pback_array(&tok_vec, &token);
                    break;
                }

                case '/':{
                    token = (token_t){
                        .type = TOK_DIV,
                        .line = line_count
                    };
                    pback_array(&tok_vec, &token);
                    break;
                }
                
                case '%':{
                    token = (token_t){
                        .type = TOK_MOD,
                        .line = line_count
                    };
                    pback_array(&tok_vec, &token);
                    break;
                }
                    
                default:{
                    fprintf(stderr, "%s.%lu: Unrecognized Token '%c'\n", fpath, line_count, c);
                    exit(-1);
                }
            }
        }
    }

    free(line_str);
    return tok_vec;
}

token_type_t is_keyword(const char* str, size_t* increment_counter){
    if (!strncmp("int", str, 3)){
        *increment_counter += 2;
        return KEYW_int;
    } else
    if (!strncmp("return", str, 6)){
        *increment_counter += 5;
        return KEYW_return;
    }
    else
        return TOK_NULL;
}

void token_dstr(void* token){
    return;
}