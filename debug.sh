gcc lexer/lexer.c parser/parser.c ir_gen/ir_gen.c code_gen/code_gen.c lib/vector/C_vector.c typecheck/typecheck.c main.c -g -lm -fsanitize=address -o main
./main code.c code.c.s
rm -r main.dSYM