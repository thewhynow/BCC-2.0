.globl _main
_main:
pushq %rbp
movq %rsp, %rbp
subq $4, %rsp
movl $1, %r10d
cmpl $0, %r10d
jne L0
movl $0, %r10d
cmpl $0, %r10d
jne L0
L0:
movl $1, -4(%rbp)
L1:
movl -4(%rbp), %eax
movq %rbp, %rsp
popq %rbp
ret
movq %rbp, %rsp
popq %rbp
ret
