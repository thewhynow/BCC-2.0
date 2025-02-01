.text
.globl _main
_main:
pushq %rbp
movq %rsp, %rbp
subq $128, %rsp
.cstring
.string_0:
.asciz "I pledge alegiance"
.text
leaq .string_0(%rip), %rax
movq %rax, -72(%rbp)
.cstring
.string_1:
.asciz "to the flag"
.text
leaq .string_1(%rip), %rax
movq %rax, -64(%rbp)
.cstring
.string_2:
.asciz "of the United States of America"
.text
leaq .string_2(%rip), %rax
movq %rax, -56(%rbp)
.cstring
.string_3:
.asciz "and to the republic"
.text
leaq .string_3(%rip), %rax
movq %rax, -48(%rbp)
.cstring
.string_4:
.asciz "for which it stands"
.text
leaq .string_4(%rip), %rax
movq %rax, -40(%rbp)
.cstring
.string_5:
.asciz "one Nation under God"
.text
leaq .string_5(%rip), %rax
movq %rax, -32(%rbp)
.cstring
.string_6:
.asciz "indivisible"
.text
leaq .string_6(%rip), %rax
movq %rax, -24(%rbp)
.cstring
.string_7:
.asciz "liberty and justice"
.text
leaq .string_7(%rip), %rax
movq %rax, -16(%rbp)
.cstring
.string_8:
.asciz "for all."
.text
leaq .string_8(%rip), %rax
movq %rax, -8(%rbp)
movq $0, -80(%rbp)
L2:
cmpq $9, -80(%rbp)
movl $0, -84(%rbp)
setb -84(%rbp)
cmpl $0, -84(%rbp)
je L1
pushq %rdi
pushq %rsi
pushq %rdx
pushq %rcx
pushq %r8
pushq %r9
pushq %r10
pushq %r11
subq $0, %rsp
.cstring
.string_9:
.asciz "%s\n"
.text
leaq .string_9(%rip), %rax
movq %rax, %rdi
leaq -72(%rbp), %rax
movq %rax, %rax
movq -80(%rbp), %rdx
leaq (%rax, %rdx, 8), %rax
movq %rax, -92(%rbp)
movq -92(%rbp), %rax
movq (%rax), %rsi
call _printf
addq $0, %rsp
pop %r11
pop %r10
pop %r9
pop %r8
pop %rcx
pop %rdx
pop %rsi
pop %rdi
pushq %rdi
pushq %rsi
pushq %rdx
pushq %rcx
pushq %r8
pushq %r9
pushq %r10
pushq %r11
subq $0, %rsp
movl $2, %edi
call _sleep
addq $0, %rsp
pop %r11
pop %r10
pop %r9
pop %r8
pop %rcx
pop %rdx
pop %rsi
pop %rdi
L0:
incq -80(%rbp)
jmp L2
L1:
movq $0, -100(%rbp)
L5:
cmpq $9, -100(%rbp)
movl $0, -104(%rbp)
setb -104(%rbp)
cmpl $0, -104(%rbp)
je L4
pushq %rdi
pushq %rsi
pushq %rdx
pushq %rcx
pushq %r8
pushq %r9
pushq %r10
pushq %r11
subq $0, %rsp
.cstring
.string_10:
.asciz "%p\n"
.text
leaq .string_10(%rip), %rax
movq %rax, %rdi
leaq -72(%rbp), %rax
movq %rax, %rax
movq -100(%rbp), %rdx
leaq (%rax, %rdx, 8), %rax
movq %rax, -112(%rbp)
movq -112(%rbp), %rsi
call _printf
addq $0, %rsp
pop %r11
pop %r10
pop %r9
pop %r8
pop %rcx
pop %rdx
pop %rsi
pop %rdi
L3:
incq -100(%rbp)
jmp L5
L4:
movl $0, %eax
movq %rbp, %rsp
popq %rbp
ret
