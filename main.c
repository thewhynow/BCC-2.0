#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ir_gen/ir_gen.h"
#include "code_gen/code_gen.h"
#include "typecheck/typecheck.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
    if (argc == 3){
        token_t* tokens = lex(argv[1]);
        node_t* tree = parse(tokens, argv[1]);
        free_array(tokens, NULL);
        typecheck(tree);
        ir_node_t* ir_nodes = ir_gen(tree);
        free_array(tree, parse_node_dstr);
        codegen(ir_nodes, argv[2]);
        free_array(ir_nodes, ir_node_dstr);
    }
    else {
        token_t* tokens = lex("code.txt");
        node_t* tree = parse(tokens, "code.txt");
        free_array(tokens, NULL);
        typecheck(tree);
        ir_node_t* ir_nodes = ir_gen(tree);
        free_array(tree, parse_node_dstr);
        codegen(ir_nodes, "code.txt.s");
        free_array(ir_nodes, ir_node_dstr);
    }

    extern char** all_identifier_strings;
    for (size_t i = 0; i < get_count_array(all_identifier_strings); ++i)
        free(all_identifier_strings[i]);

    free_array(all_identifier_strings, NULL);
}