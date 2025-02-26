BCC - Bad C Compiler
    a small side project of mine I started in November '24. Feel free to take a look around!
    for those interested in projects like these: the book "Writing A C Compiler" by Nora Sandler

run the "run.sh" file to compile the c-file "code.c" into assembly file "code.c.s" and into executeable "code"

FEATURES SUPPORTED:
    1. all c-language constructs
        - loops, functions, switches, etc.
    2. static and file-scope variables
        - can be initialized with number constants or constant literals
            - typecasts of constant expressions are now considered constant
    3. types: char, short, int, long + unsigned for all, + arrays, pointers, strings, and STRUCTS!
        - no implicit casts -> even for constants (excluding array to pointer casts)
        - to declare a char, short, or long constant use 'C', 'S', or 'L' suffixes respectively
        - to declare an unsigned constant, use the 'U' suffix AFTER the base-type suffix
        - for structure literals, must use the {.identifier = value} syntax


PARTS OF THE C-STANDARD IGNORED:
    - allowing function, struct, and symbol declarations inside scope
    ** there are a crap ton more not followed but these are the commonly used ones **

TO COMPILE YOUR OWN CODE:
    1. copy and paste the first line of the "run.sh" file into the terminal
    2. hit "enter"
    3. enter the command "./main <c_file> <asm_file>"
        - <c_file> -> the c-file you want to compile
        - <asm_file> -> the file that you want to output assembly too
    4. to assemble the outputed assembly from the compiler, do "gcc -x assembler <asm_file> -o <exe_file>"
        - <exe_file> -> the path that you want the executeable to be outputted to