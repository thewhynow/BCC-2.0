#include "lexer.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
# include <Windows.h>
static inline size_t getpagesize(){
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwPageSize;
}
#else
# include <unistd.h> // sysconf(3)
# define getpagesize() sysconf(_SC_PAGE_SIZE)
#endif

/* return TOK_NULL (0) on false */
token_type_t is_keyword(const char* str, size_t* increment_counter);



token_t* lex(const char* fpath){
    /* neat little trick i learned. the '+' indicates to only open an existing file */
    FILE* c_file = fopen(fpath, "r+");

    if (!c_file){
        fprintf(stderr, "%s: Unable to open file", fpath);
        exit(-1);
    }

    char* line_str = malloc(sizeof(char) * getpagesize());

    if (!line_str){
        fprintf(stderr, "%s: Ran out of memory while compiling", fpath);
        exit(-1);
    }

    size_t line_count = 1;

    token_t* tok_vec = alloc_array(sizeof(token_t), 1);

    token_t token;

    while (!feof(c_file)){
        memset(line_str, 0, getpagesize());
        if (!fread(line_str, 1, getpagesize(), c_file)){
            fprintf(stderr, "%s.%lu: Error reading file\n", fpath, line_count);
            exit(-1);
        }

        size_t line_str_len = strlen(line_str);
        
        for (size_t i = 0; i < line_str_len; ++i){
            char c = line_str[i];
            switch(c){
                case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': {
                    unsigned long long num = 0;

                    while ('0' <= c && c <= '9'){
                        num *= 10;
                        num += c - 48;
                        c = line_str[++i];
                    }

                    token = (token_t){
                        .line = line_count,
                    };

                    if (line_str[i] == 'L' || line_str[i] == 'l'){
                        token.type = TOK_LONG;
                        token.literals.ulong = num;
                    } else
                    /* these two integer-constant-suffixes are temporary */
                    if (line_str[i] == 'S' || line_str[i] == 's'){
                        token.type = TOK_SHORT;
                        token.literals.ushort = num;
                    } else
                    if (line_str[i] == 'C' || line_str[i] == 'c'){
                        token.type = TOK_CHAR;
                        token.literals.uchar = num;
                    }
                    else {
                        token.type = TOK_INTEGER;
                        token.literals.uint = num;
                        --i;
                    }

                    if (line_str[i + 1] == 'U' || line_str[i + 1] == 'u'){
                        token.type += 4;
                        ++i;
                    }

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

                        if (line_str[i + 1] == ':'){
                            tok_vec[get_count_array(tok_vec) - 1].type = TOK_LABEL;
                            ++i;
                        }

                        break;
                    }

                }

                case '-':{
                    if ((i < (line_str_len - 1)) && (line_str[i + 1] == '=')) {
                        token = (token_t){
                            .type = TOK_SUB_ASSIGN,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                        ++i;
                    }
                    else {
                        token = (token_t){
                            .type = TOK_SUB,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                    }

                    break;
                }

                case '!':{
                    if (line_str[i + 1] == '='){
                        ++i;
                        token = (token_t){
                            .type = TOK_NOT_EQUAL_TO,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                    }
                    else {
                        token = (token_t){
                            .type = TOK_LOGICAL_NOT,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                    }
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
                    if ((i < (line_str_len - 1)) && (line_str[i + 1] == '=')) {
                        token = (token_t){
                            .type = TOK_ADD_ASSIGN,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                        ++i;
                    } else 
                    if ((i < (line_str_len - 1)) && (line_str[i + 1] == '+')) {
                        token = (token_t){
                            .type = TOK_INCREMENT,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                        ++i;
                    }
                    else {
                        token = (token_t){
                            .type = TOK_ADD,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                    }
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
                        if ((i < (line_str_len - 1)) && (line_str[i + 1] == '=')){
                            ++i;
                            token = (token_t){
                                .type = TOK_AND_ASSIGN,
                                .line = line_count
                            };
                            pback_array(&tok_vec, &token);
                        }
                        else {
                            token = (token_t){
                                .type = TOK_BITWISE_AND,
                                .line = line_count
                            };
                            pback_array(&tok_vec, &token);
                        }
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
                        if ((i < (line_str_len - 1)) && (line_str[i + 1] == '=')){
                            ++i;
                            token = (token_t){
                                .type = TOK_ORR_ASSIGN,
                                .line = line_count
                            };
                            pback_array(&tok_vec, &token);
                        }
                        else {
                            token = (token_t){
                                .type = TOK_BITWISE_OR,
                                .line = line_count
                            };
                            pback_array(&tok_vec, &token);
                        }
                    }
                    break;
                }
                
                case '^':{
                    if ((i < (line_str_len - 1)) && (line_str[i + 1] == '=')){
                            ++i;
                            token = (token_t){
                                .type = TOK_XOR_ASSIGN,
                                .line = line_count
                            };
                            pback_array(&tok_vec, &token);
                        }
                        else {
                            token = (token_t){
                                .type = TOK_BITWISE_XOR,
                                .line = line_count
                            };
                            pback_array(&tok_vec, &token);
                        }
                    break;
                }

                case '<':{
                    if ((i < (line_str_len - 1)) && (line_str[i + 1] == '<')){
                        ++i;
                        if ((i < (line_str_len - 1)) && (line_str[i + 1] == '=')){
                            ++i;
                            token = (token_t){
                                .type = TOK_LSH_ASSIGN,
                                .line = line_count
                            };
                            pback_array(&tok_vec, &token);
                        }
                        else {
                            token = (token_t){
                                .type = TOK_SHIFT_LEFT,
                                .line = line_count
                            };
                            pback_array(&tok_vec, &token);
                        }
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
                        ++i;
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
                    if ((i < (line_str_len - 1)) && (line_str[i + 1] == '=')) {
                        token = (token_t){
                            .type = TOK_MUL_ASSIGN,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                        ++i;
                    }
                    else {
                        token = (token_t){
                            .type = TOK_MUL,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                    }
                    break;
                }

                case '/':{
                    if ((i < (line_str_len - 1)) && (line_str[i + 1] == '=')) {
                        token = (token_t){
                            .type = TOK_DIV_ASSIGN,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                        ++i;
                    }
                    else {
                        token = (token_t){
                            .type = TOK_DIV,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                    }
                    break;
                }
                
                case '%':{
                    if ((i < (line_str_len - 1)) && (line_str[i + 1] == '=')) {
                        token = (token_t){
                            .type = TOK_MOD_ASSIGN,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                        ++i;
                    }
                    else {
                        token = (token_t){
                            .type = TOK_MOD,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                    }
                    break;
                }

                case '=': {
                    if ((i < (line_str_len - 1)) && (line_str[i + 1] == '=')) {
                        token = (token_t){
                            .type = TOK_EQUAL_TO,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                        ++i;
                    }
                    else {
                        token = (token_t){
                            .type = TOK_ASSIGN,
                            .line = line_count
                        };
                        pback_array(&tok_vec, &token);
                    }
                    break;
                }

                case '?': {
                    token = (token_t){
                        .type = TOK_QUESTION_MARK,
                        .line = line_count
                    };
                    pback_array(&tok_vec, &token);
                    break;
                }

                case ':': {
                    token = (token_t){
                        .type = TOK_COLON,
                        .line = line_count
                    };
                    pback_array(&tok_vec, &token);
                    break;
                }

                case ',': {
                    token = (token_t){
                        .type = TOK_COMMA,
                        .line = line_count
                    };
                    pback_array(&tok_vec, &token);
                    break;
                }

                case '\'': {
                    token = (token_t){
                        .type = TOK_INTEGER,
                        .literals.uint = line_str[++i],
                        .line = line_count
                    };
                    pback_array(&tok_vec, &token);
                    ++i;
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
    
    fclose(c_file);

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
    } else
    if (!strncmp("if", str, 2)){
        ++*increment_counter;
        return KEYW_if;
    } else
    if (!strncmp("else", str, 4)){
        *increment_counter += 3;
        return KEYW_else;
    } else
    if (!strncmp("while", str, 5)){
        *increment_counter += 4;
        return KEYW_while;
    } else
    if (!strncmp("break", str, 5)){
        *increment_counter += 4;
        return KEYW_break;
    } else
    if (!strncmp("continue", str, 8)){
        *increment_counter += 7;
        return KEYW_continue;
    } else
    if (!strncmp("do", str, 2)){
        *increment_counter += 1;
        return KEYW_do;
    } else
    if (!strncmp("switch", str, 6)){
        *increment_counter += 5;
        return KEYW_switch;
    } else
    if (!strncmp("for", str, 3)){
        *increment_counter += 2;
        return KEYW_for;
    } else
    if (!strncmp("goto", str, 4)){
        *increment_counter += 3;
        return KEYW_goto;
    } else
    if (!strncmp("case", str, 4)){
        *increment_counter += 3;
        return KEYW_case;
    } else
    if (!strncmp("default", str, 7)){
        *increment_counter += 6;
        return KEYW_default;
    } else
    if (!strncmp("static", str, 6)){
        *increment_counter += 5;
        return KEYW_static;
    } else
    if (!strncmp("extern", str, 6)){
        *increment_counter += 5;
        return KEYW_extern;
    } else
    if (!strncmp("long", str, 4)){
        *increment_counter += 3;
        return KEYW_long;
    } else
    if (!strncmp("short", str, 5)){
        *increment_counter += 4;
        return KEYW_short;
    } else
    if (!strncmp("char", str, 4)){
        *increment_counter += 3;
        return KEYW_char;
    } else
    if (!strncmp("unsigned", str, 8)){
        *increment_counter += 7;
        return KEYW_unsigned;
    } else
    if (!strncmp("signed", str, 6)){
        *increment_counter += 5;
        return KEYW_signed;
    }
    else
        return TOK_NULL;
}