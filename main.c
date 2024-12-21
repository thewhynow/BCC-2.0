#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ir_gen/ir_gen.h"
#include "code_gen/code_gen.h"

#include <stdio.h>
int main(int argc, char** argv){
    token_t* tokens = lex("code.c");
    node_t* tree = parse(tokens, "code.c");
    free_array(tokens, token_dstr);
    ir_func_t* ir_nodes = ir_gen(tree);
    free_array(tree, parse_node_dstr);
    codegen(ir_nodes, "code.c.s");
    free_array(ir_nodes, ir_node_dstr);
}