.text
.globl _print_three
_print_three:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movl %edi, -7(%rbp)
shrq $32, %rdi
movw %di, -3(%rbp)
shrq $16, %rdi
movb %dil, -1(%rbp)
pushq %rdi
pushq %rsi
pushq %rdx
pushq %rcx
pushq %r8
pushq %r9
pushq %r10
pushq %r11
leaq .string_0(%rip), %r11
movq %r11, %rdi
leaq -7(%rbp), %r11
movq %r11, %rsi
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
movl $0, %eax
movq %rbp, %rsp
popq %rbp
ret
.globl _main
_main:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
pushq %rdi
pushq %rsi
pushq %rdx
pushq %rcx
pushq %r8
pushq %r9
pushq %r10
pushq %r11
movb $97, -7(%rbp)
movb $98, -6(%rbp)
movb $99, -5(%rbp)
movb $100, -4(%rbp)
movb $101, -3(%rbp)
movb $102, -2(%rbp)
movb $0, -1(%rbp)
movq -7(%rbp), %r10
movq %r10, %rdi
call _print_three
addq $0, %rsp
pop %r11
pop %r10
pop %r9
pop %r8
pop %rcx
pop %rdx
pop %rsi
pop %rdi
movl $0, %eax
movq %rbp, %rsp
popq %rbp
ret
.data
.globl _idk
_idk:
.balign 16
.long 1
.byte 97
.byte 98
.byte 99
.zero 1
.long 2
.cstring
.string_0:
.asciz "%s\n"
.bss
