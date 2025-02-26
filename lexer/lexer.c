#include "lexer.h"

/* return TOK_NULL (0) on false */
token_type_t is_keyword(const char* str, size_t* increment_counter);

char** all_identifier_strings;

token_t* lex(const char* fpath){
    /* neat little trick i learned. the '+' indicates to only open an existing file */
    void* c_file = fopen(fpath, "r+");

    if (!c_file){
        fprintf(stderr, "%s: Unable to open file", fpath);
        exit(-1);
    }

    fseek(c_file, 0, 2);

    long file_len = ftell(c_file);

    rewind(c_file);

    char* line_str = malloc(sizeof(char) * file_len + 1);

    if (!line_str){
        fprintf(stderr, "%s: Ran out of memory while compiling", fpath);
        exit(-1);
    }
    
    fread(line_str, 1, file_len, c_file);
    line_str[file_len] = '\0';

    size_t line_count = 1;

    token_t* tok_vec = alloc_array(sizeof(token_t), 1);

    token_t token;

    all_identifier_strings = alloc_array(sizeof(char*), 1);

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
                    if (keyw_type != TOK_BYPASS)
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

                    pback_array(&all_identifier_strings, &identifier);

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
                } else
                if ((i < (line_str_len - 1)) && (line_str[i + 1] == '>')) {
                    token = (token_t){
                        .type = TOK_ARROW,
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
                } else
                if ((i < (line_str_len - 1)) && (line_str[i + 1] == '/')) {
                    do
                        ++i;
                    while (line_str[i] != '\n');
                } else
                if ((i < (line_str_len - 1)) && (line_str[i + 1] == '*')){
                    ++i;
                    while (true){
                        if (line_str[i] == '\n')
                            ++line_count;
                        ++i;
                        if (line_str[i] == '*' && line_str[i + 1] == '/'){
                            i += 2;
                            break;
                        }
                    }
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
                ++i;
                token.type = TOK_CHAR;
                token.line = line_count;

                if (line_str[i] == '\\'){
                    switch(line_str[++i]){
                        case '0':
                            token.literals.uchar = '\0';
                            break;
                        case '\\':
                            token.literals.uchar = '\\';
                            break;
                        case 'n':
                            token.literals.uchar = '\n';
                            break;
                        case '\'':
                            token.literals.uchar = '\'';
                            break;
                        case '\"':
                            token.literals.uchar = '\"';
                            break;
                        case '?':
                            token.literals.uchar = '?';
                            break;
                        case 'a':
                            token.literals.uchar = '\a';
                            break;
                        case 'b':
                            token.literals.uchar = '\b';
                            break;
                        case 'f':
                            token.literals.uchar = '\f';
                            break;
                        case 'r':
                            token.literals.uchar = '\r';
                            break;
                        case 't':
                            token.literals.uchar = '\t';
                            break;
                        case 'v':
                            token.literals.uchar = '\v';
                            break;

                        default:
                            fprintf(stderr, "%s.%lu: unrecognized escape sequence\n", fpath, line_count);
                            exit(-1);
                    }
                }
                else {
                    token.literals.uchar = line_str[i];
                }

                pback_array(&tok_vec, &token);
                ++i;
                break;
            }

            case '[': {
                token = (token_t){
                    .type = TOK_OPEN_BRACKET,
                    .line = line_count
                };
                pback_array(&tok_vec, &token);
                break;
            }
            
            case ']': {
                token = (token_t){
                    .type = TOK_CLOSE_BRACKET,
                    .line = line_count
                };
                pback_array(&tok_vec, &token);
                break;
            }

            case '\"': {
                char* str = alloc_array(sizeof(char), 1);

                char c;

                while (line_str[i] == '\"'){
                    ++i;
                    for (; line_str[i] != '\"'; ++i){
                        if (line_str[i] == '\\')
                            switch(line_str[++i]){
                                case '0':
                                    c = '\0';
                                    break;
                                case '\\':
                                    c = '\\';
                                    break;
                                case 'n':
                                    c = '\n';
                                    break;
                                case '\'':
                                    c = '\'';
                                    break;
                                case '\"':
                                    c = '\"';
                                    break;
                                case '?':
                                    c = '?';
                                    break;
                                case 'a':
                                    c = '\a';
                                    break;
                                case 'b':
                                    c = '\b';
                                    break;
                                case 'f':
                                    c = '\f';
                                    break;
                                case 'r':
                                    c = '\r';
                                    break;
                                case 't':
                                    c = '\t';
                                    break;
                                case 'v':
                                    c = '\v';
                                    break;

                                default:
                                    fprintf(stderr, "%s.%lu: unrecognized escape sequence\n", fpath, line_count);
                                    exit(-1);
                            }
                        else
                            c = line_str[i];

                        pback_array(&str, &c);
                    }
                    ++i;

                    while (line_str[i] == '\n' || line_str[i] == '\t' || line_str[i] == ' ')
                        ++i;
                }

                --i;

                c = '\0';

                pback_array(&str, &c);

                token = (token_t){
                    .type = TOK_STRING_LITERAL,
                    .identifier = strdup(str)
                };
                pback_array(&tok_vec, &token);

                pback_array(&all_identifier_strings, &token.identifier);

                free_array(str, NULL);

                break;
            }

            case '.': {
                if (line_str[i + 1] == '.' && line_str[i + 2] == '.'){
                    i += 2;

                    token = (token_t){
                        .type = TOK_ELIPSES,
                    };
                    pback_array(&tok_vec, &token);
                }
                else {
                    token = (token_t){
                        .type = TOK_PERIOD
                    };
                    pback_array(&tok_vec, &token);
                }

                break;
            }

            default:{
                fprintf(stderr, "%s.%lu: Unrecognized Token '%c'\n", fpath, line_count, c);
                exit(-1);
            }
        }
    }

    free(line_str);
    
    fclose(c_file);

    return tok_vec;
}

token_type_t is_keyword(const char* str, size_t* increment_counter){
    int keyword_strcmp(const char* keyword, const char* str, size_t N);

    if (!keyword_strcmp("int", str, 3)){
        *increment_counter += 2;
        return KEYW_int;
    } else
    if (!keyword_strcmp("return", str, 6)){
        *increment_counter += 5;
        return KEYW_return;
    } else
    if (!keyword_strcmp("if", str, 2)){
        ++*increment_counter;
        return KEYW_if;
    } else
    if (!keyword_strcmp("else", str, 4)){
        *increment_counter += 3;
        return KEYW_else;
    } else
    if (!keyword_strcmp("while", str, 5)){
        *increment_counter += 4;
        return KEYW_while;
    } else
    if (!keyword_strcmp("break", str, 5)){
        *increment_counter += 4;
        return KEYW_break;
    } else
    if (!keyword_strcmp("continue", str, 8)){
        *increment_counter += 7;
        return KEYW_continue;
    } else
    if (!keyword_strcmp("do", str, 2)){
        *increment_counter += 1;
        return KEYW_do;
    } else
    if (!keyword_strcmp("switch", str, 6)){
        *increment_counter += 5;
        return KEYW_switch;
    } else
    if (!keyword_strcmp("for", str, 3)){
        *increment_counter += 2;
        return KEYW_for;
    } else
    if (!keyword_strcmp("goto", str, 4)){
        *increment_counter += 3;
        return KEYW_goto;
    } else
    if (!keyword_strcmp("case", str, 4)){
        *increment_counter += 3;
        return KEYW_case;
    } else
    if (!keyword_strcmp("default", str, 7)){
        *increment_counter += 6;
        return KEYW_default;
    } else
    if (!keyword_strcmp("static", str, 6)){
        *increment_counter += 5;
        return KEYW_static;
    } else
    if (!keyword_strcmp("extern", str, 6)){
        *increment_counter += 5;
        return KEYW_extern;
    } else
    if (!keyword_strcmp("long", str, 4)){
        *increment_counter += 3;
        return KEYW_long;
    } else
    if (!keyword_strcmp("short", str, 5)){
        *increment_counter += 4;
        return KEYW_short;
    } else
    if (!keyword_strcmp("char", str, 4)){
        *increment_counter += 3;
        return KEYW_char;
    } else
    if (!keyword_strcmp("unsigned", str, 8)){
        *increment_counter += 7;
        return KEYW_unsigned;
    } else
    if (!keyword_strcmp("signed", str, 6)){
        *increment_counter += 5;
        return KEYW_signed;
    } else
    if (!keyword_strcmp("restrict", str, 8)){
        *increment_counter += 7;
        #if WARN_UNSUPPORTED_KEYWORDS
            fprintf(stderr, "WARNING: 'restrict' keyword found and ignored\n");
        #endif
        return TOK_BYPASS; /* ignore the keyword */
    } else
    if (!keyword_strcmp("const", str, 5)){
        *increment_counter += 4;
        #if WARN_UNSUPPORTED_KEYWORDS
            fprintf(stderr, "WARNING: 'const' keyword found and ignored\n");
        #endif
        return TOK_BYPASS; /* ignore the keyword */
    } else
    if (!keyword_strcmp("void", str, 4)){
        *increment_counter += 3;
        return KEYW_void;
    } else
    if (!keyword_strcmp("sizeof", str, 6)){
        *increment_counter += 5;
        return KEYW_sizeof;
    } else
    if (!keyword_strcmp("struct", str, 6)){
        *increment_counter += 5;
        return KEYW_struct;
    } else
    if (!keyword_strcmp("typedef", str, 7)){
        *increment_counter += 6;
        return KEYW_typedef;
    } else
    if (!keyword_strcmp("union", str, 5)){
        *increment_counter += 4;
        return KEYW_union;
    }
    else
        return TOK_NULL;
}

int keyword_strcmp(const char* keyword, const char* str, size_t N){
    int result = strncmp(keyword, str, strlen(keyword));

    if (strlen(str) > N && is_identifier_char(str[N]))
        return 1;
    else
        return result;
}