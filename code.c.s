.text
.globl _add
_add:
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
.globl _main
_main:
pushq %rbp
movq %rsp, %rbp
subq $0, %rsp
subq $0, %rsp
movq $1, %rdi
movb $1, %sil
movw $0, %dx
movl $5, %ecx
movq $0, %r8
movb $9, %r9b
pushq $10
pushq $0
pushq $0
pushq $1234567
pushq $0
call _add
addq $40, %rsp
movl %eax, %eax
movq %rbp, %rsp
popq %rbp
ret
movl $0, %eax
movq %rbp, %rsp
popq %rbp
ret
