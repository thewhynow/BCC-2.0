gcc lexer/lexer.c parser/parser.c ir_gen/ir_gen.c code_gen/code_gen.c lib/vector/C_vector.c typecheck/typecheck.c main.c -lm -o main -Wall
./main code.c code.c.s
os_type=$(uname)

# ai generated code so sue me i dont know bash

if [[ "$os_type" == "Linux" ]]; then
    gcc code.c.s -o code -fPIE
elif [[ "$os_type" == "Darwin" ]]; then
    gcc code.c.s -arch x86_64 -o code
else
    echo "Unknown OS: $os_type"
fi

rm main