gcc lexer/lexer.c parser/parser.c ir_gen/ir_gen.c code_gen/code_gen.c lib/vector/C_vector.c main.c -g -lm -fsanitize=address -o main
./main