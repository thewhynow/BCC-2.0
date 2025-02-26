#include "lib/my_vec.c"

#define TOK_EOF 0
#define TOK_INT 1
#define TOK_ADD 2
#define TOK_SUB 3
#define TOK_MUL 4
#define TOK_DIV 5

typedef struct {
    int type;
    int val;
} token_t;

token_t* lex_buff(const char* buff){
    token_t* tokens = alloc_array(sizeof *tokens, 1LU);

    token_t curr_token;

    unsigned long i = 0LU;

    unsigned long buff_len = strlen(buff);

    while (i < buff_len){
        switch((int)buff[i]){
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {            
                curr_token = (token_t){.type = TOK_INT, .val = 0};

                while ((char)'0' <= buff[i] && buff[i] <= (char)'9'){
                    curr_token.val = curr_token.val * 10 + (int)(buff[i] - (char)'0');
                    ++i;
                }
                
                pback_array(&tokens, &curr_token);
                break;
            }

            case '+': {
                curr_token.type = TOK_ADD;
                pback_array(&tokens, &curr_token);
                ++i;
                break;
            }

            case '-': {
                curr_token.type = TOK_SUB;
                pback_array(&tokens, &curr_token);
                ++i;
                break;
            }

            case '*': {
                curr_token.type = TOK_MUL;
                pback_array(&tokens, &curr_token);
                ++i;
                break;
            }

            case '/': {
                curr_token.type = TOK_DIV;
                pback_array(&tokens, &curr_token);
                ++i;
                break;
            }
        }
    }

    curr_token.type = TOK_EOF;
    pback_array((void**)&tokens, &curr_token);

    return tokens;
}

#define NOD_NUL 0
#define NOD_INT 1
#define NOD_ADD 2
#define NOD_SUB 3
#define NOD_MUL 4
#define NOD_DIV 5

typedef struct node_t node_t;

struct node_t {
    int type;
    int val;
    node_t* val_a;
    node_t* val_b;
};

static token_t* curr_token;

void advance_token(){
    static unsigned long token_count = 0LU;
    if (curr_token->type != TOK_EOF){
        curr_token += 1LU;
        ++token_count;
    }
}

node_t* alloc_node(node_t n){
    node_t* res = malloc(sizeof n);
    *res = n;
    return res;
}

node_t parse_fac(){
    node_t node;
    switch(curr_token->type){
        case TOK_INT: {
            node.type = NOD_INT;
            node.val = curr_token->val;
            advance_token();
            return node;
        }

        default: {
            printf("expected int\n");
            exit(-1);
        }
    }
}

node_t parse_term(){
    node_t result = parse_fac();
    while (true){
        switch(curr_token->type){
            case TOK_MUL: {
                advance_token();

                result = (node_t){
                    .type = NOD_MUL,
                    .val_a = alloc_node(result),
                    .val_b = alloc_node(parse_fac())
                };

                break;
            }

            case TOK_DIV: {
                advance_token();

                result = (node_t){
                    .type = NOD_DIV,
                    .val_a = alloc_node(result),
                    .val_b = alloc_node(parse_fac())
                };

                break;
            }

            default: {
                return result;
            }
        }
    }
}

node_t parse_expr(){
    node_t result = parse_term();
    while(true){
        switch(curr_token->type){
            case TOK_ADD: {
                advance_token();

                result = (node_t){
                    .type = NOD_ADD,
                    .val_a = alloc_node(result),
                    .val_b = alloc_node(parse_term())
                };

                break;
            }

            case TOK_SUB: {
                advance_token();

                result = (node_t){
                    .type = NOD_SUB,
                    .val_a = alloc_node(result),
                    .val_b = alloc_node(parse_term())
                };

                break;
            }

            default: {
                return result;
            }
        }
    }
}

node_t parse(token_t* tokens){
    curr_token = tokens;

    return parse_expr();
}

int interpret(node_t node){
    switch(node.type){
        case NOD_INT: {
            return node.val;
        }

        case NOD_ADD: {
            return interpret(*node.val_a) + interpret(*node.val_b);
        }

        case NOD_SUB: {
            return interpret(*node.val_a) - interpret(*node.val_b);
        }

        case NOD_MUL: {
            return interpret(*node.val_a) * interpret(*node.val_b);
        }
        
        case NOD_DIV: {
            return interpret(*node.val_a) / interpret(*node.val_b);
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    if (argc == 2){
        printf("%i\n", interpret(parse(lex_buff(argv[1LU]))));
    }
    else
        puts("enter argument");
}