.text
.globl add
add:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movq %rdi, -8(%rbp)
movq 48(%rbp), %r10
addq %r10, -8(%rbp)
movq -8(%rbp), %rax
movq %rbp, %rsp
popq %rbp
ret
movl $0, %eax
movq %rbp, %rsp
popq %rbp
ret
.globl sub
sub:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movl %edi, -4(%rbp)
subl %esi, -4(%rbp)
movl -4(%rbp), %eax
movq %rbp, %rsp
popq %rbp
ret
movl $0, %eax
movq %rbp, %rsp
popq %rbp
ret
.globl main
main:
pushq %rbp
movq %rsp, %rbp
subq $48, %rsp
movl $12, -4(%rbp)
movl $1, -12(%rbp)
movq $0, -24(%rbp)
L2:
cmpq $10, -24(%rbp)
movl $0, -36(%rbp)
setb -36(%rbp)
cmpl $0, -36(%rbp)
je L1
movl $12, %eax
movq %rbp, %rsp
popq %rbp
ret
L0:
movq -24(%rbp), %rax
movq %rax, -44(%rbp)
addq $1, -44(%rbp)
movq -44(%rbp), %rax
movq %rax, -24(%rbp)
jmp L2
L1:
movl $0, %eax
movq %rbp, %rsp
popq %rbp
ret
.section .note.GNU-stack, "",@progbits
