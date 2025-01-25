.text
.globl _main
_main:
pushq %rbp
movq %rsp, %rbp
subq $112, %rsp
movq $1, -32(%rbp)
movq $2, -24(%rbp)
movq $3, -16(%rbp)
movq $4, -8(%rbp)
lea -32(%rbp), %rax
movq %rax, -40(%rbp)
movq -40(%rbp), %rax
movq $1, %rdx
movq %rdx, %r10
movq $16, %r11
imulq %r11, %r10
movq %r10, %r10
lea (%rax, %r10, 1), %rax
movq %rax, -48(%rbp)
movq -48(%rbp), %rax
lea (%rax), %rax
movq %rax, -56(%rbp)
movq -56(%rbp), %rax
movq $0, %rdx
lea (%rax, %rdx, 8), %rax
movq %rax, -64(%rbp)
movq -64(%rbp), %rax
movq $12, (%rax)
lea -32(%rbp), %rax
movq %rax, -72(%rbp)
movq -72(%rbp), %rax
movq $1, %rdx
movq %rdx, %r10
movq $16, %r11
imulq %r11, %r10
movq %r10, %r10
lea (%rax, %r10, 1), %rax
movq %rax, -80(%rbp)
movq -80(%rbp), %rax
lea (%rax), %rax
movq %rax, -88(%rbp)
movq -88(%rbp), %rax
movq $0, %rdx
lea (%rax, %rdx, 8), %rax
movq %rax, -96(%rbp)
movq -96(%rbp), %rax
movl (%rax), %eax
movq %rbp, %rsp
popq %rbp
ret
movl $0, %eax
movq %rbp, %rsp
popq %rbp
ret
