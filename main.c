#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ir_gen/ir_gen.h"
#include "code_gen/code_gen.h"
#include "typecheck/typecheck.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int main(int argc, char** argv){
    const char* c_file_name;
    const char* out_file_name;
    
    if (argc == 3){
        c_file_name = argv[1];
        out_file_name = argv[2];
    }
    else {
        c_file_name = "code.c";
        out_file_name = "code.c.s";
    }

    static char command[4096];
    snprintf(command, 4096, "gcc -E -P %s -o %s", c_file_name, out_file_name);

    if (system(command))
        fprintf(stderr, "failed to preprocess file: %s\n", c_file_name);

    token_t* tokens = lex(out_file_name);
    node_t* tree = parse(tokens, out_file_name);
    free_array(tokens, NULL);
    typecheck(tree);
    ir_node_t* ir_nodes = ir_gen(tree);
    free_array(tree, parse_node_dstr);
    codegen(ir_nodes, out_file_name);
    free_array(ir_nodes, ir_node_dstr);

    extern char** all_identifier_strings;
    for (size_t i = 0; i < get_count_array(all_identifier_strings); ++i)
        free(all_identifier_strings[i]);

    free_array(all_identifier_strings, NULL);
}

