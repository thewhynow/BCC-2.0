gcc lexer/lexer.c parser/parser.c ir_gen/ir_gen.c code_gen/code_gen.c lib/vector/C_vector.c typecheck/typecheck.c main.c -lm -o main -Wall
./main code.c code.c.s

if [[ $(uname) == "Linux" ]]; then
    gcc code.c.s -o code -no-pie
else
    gcc code.c.s -arch x86_64 -o code
fi

rm main