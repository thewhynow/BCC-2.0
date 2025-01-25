.text
.bss
.balign 16
zeros:
.zero 984
.text
.data
hello_world:
.balign 1
.byte 72
.byte 101
.byte 108
.byte 108
.byte 111
.byte 44
.byte 32
.byte 87
.byte 111
.byte 114
.byte 108
.byte 100
.byte 33
.byte 0
.text
.globl main
main:
pushq %rbp
movq %rsp, %rbp
subq $64, %rsp
pushq %rdi
pushq %rsi
pushq %rdx
pushq %rcx
pushq %r8
pushq %r9
pushq %r10
pushq %r11
subq $8, %rsp
subq $0, %rsp
leaq hello_world(%rip), %rax
movq %rax, -8(%rbp)
movq -8(%rbp), %rdi
call puts
addq $0, %rsp
addq $8, %rsp
pop %r11
pop %r10
pop %r9
pop %r8
pop %rcx
pop %rdx
pop %rsi
pop %rdi
leaq zeros(%rip), %rax
movq %rax, -16(%rbp)
movq -16(%rbp), %rax
movq $12, %rdx
leaq (%rax, %rdx, 8), %rax
movq %rax, -24(%rbp)
leaq zeros(%rip), %rax
movq %rax, -32(%rbp)
movq -32(%rbp), %rax
movq $3, %rdx
leaq (%rax, %rdx, 8), %rax
movq %rax, -40(%rbp)
movq -24(%rbp), %rax
movq %rax, -48(%rbp)
movq -40(%rbp), %r10
subq %r10, -48(%rbp)
movq -48(%rbp), %rax
cdq
movq $8, %r10
divq %r10
movq %rax, -56(%rbp)
movl -56(%rbp), %eax
movq %rbp, %rsp
popq %rbp
ret
movl $0, %eax
movq %rbp, %rsp
popq %rbp
ret
.section .note.GNU-stack, "",@progbits
