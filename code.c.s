.text
.globl _main
_main:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movq $4, -8(%rbp)
addq $12, -8(%rbp)
movl -8(%rbp), %eax
movq %rbp, %rsp
popq %rbp
ret
movl $0, %eax
movq %rbp, %rsp
popq %rbp
ret
.data
.cstring
.bss
