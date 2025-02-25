if [[ $(uname) == "Linux" ]]; then
    gcc lexer/lexer.c parser/parser.c ir_gen/ir_gen.c code_gen/code_gen.c lib/vector/C_vector.c typecheck/typecheck.c main.c -g -lm -o main
    valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./main code.c code.c.s
    rm main
    gcc code.c.s -o code
else
    gcc lexer/lexer.c parser/parser.c ir_gen/ir_gen.c code_gen/code_gen.c lib/vector/C_vector.c typecheck/typecheck.c main.c -g -lm -o main -fsanitize=address,undefined
    ./main code.c code.c.s
    rm -r main.dSYM
    rm main
    gcc code.c.s -arch x86_64 -o code
fi