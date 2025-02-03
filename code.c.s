.text
.globl main
main:
pushq %rbp
movq %rsp, %rbp
subq $96, %rsp
pushq %rdi
pushq %rsi
pushq %rdx
pushq %rcx
pushq %r8
pushq %r9
pushq %r10
pushq %r11
subq $0, %rsp
leaq is_prime(%rip), %r11
movq %r11, %rdi
movl $1, %esi
movq $1000, %rdx
call memset
addq $0, %rsp
pop %r11
pop %r10
pop %r9
pop %r8
pop %rcx
pop %rdx
pop %rsi
pop %rdi
leaq is_prime(%rip), %r11
movq %r11, %rax
movq $0, %rdx
leaq (%rax, %rdx, 1), %rax
movq %rax, -8(%rbp)
movq -8(%rbp), %r11
leaq is_prime(%rip), %r11
movq %r11, %rax
movq $1, %rdx
leaq (%rax, %rdx, 1), %rax
movq %rax, -16(%rbp)
movq -16(%rbp), %r11
movb (%r11), %al
movb %al, (%r11)
movb $0, (%r11)
movl $2, -20(%rbp)
L2:
movl -20(%rbp), %r10d
imull -20(%rbp), %r10d
movl %r10d, -24(%rbp)
cmpl $999, -24(%rbp)
movl $0, -28(%rbp)
setle -28(%rbp)
cmpl $0, -28(%rbp)
je L1
leaq is_prime(%rip), %r11
movslq -20(%rbp), %rax
movq %rax, -36(%rbp)
movq %r11, %rax
movq -36(%rbp), %rdx
leaq (%rax, %rdx, 1), %rax
movq %rax, -44(%rbp)
movq -44(%rbp), %r11
cmpb $0, (%r11)
je L3
movl -20(%rbp), %r10d
imull -20(%rbp), %r10d
movl %r10d, -52(%rbp)
movl -52(%rbp), %eax
movl %eax, -48(%rbp)
L7:
cmpl $999, -48(%rbp)
movl $0, -56(%rbp)
setle -56(%rbp)
cmpl $0, -56(%rbp)
je L6
leaq is_prime(%rip), %r11
movslq -48(%rbp), %rax
movq %rax, -64(%rbp)
movq %r11, %rax
movq -64(%rbp), %rdx
leaq (%rax, %rdx, 1), %rax
movq %rax, -72(%rbp)
movq -72(%rbp), %r11
movb $0, (%r11)
L5:
movl -48(%rbp), %eax
movl %eax, -76(%rbp)
movl -20(%rbp), %r10d
addl %r10d, -76(%rbp)
movl -76(%rbp), %eax
movl %eax, -48(%rbp)
jmp L7
L6:
jmp L4
L3:
L4:
L0:
movl -20(%rbp), %eax
movl %eax, -80(%rbp)
incl -20(%rbp)
jmp L2
L1:
movl $0, %eax
movq %rbp, %rsp
popq %rbp
ret
movl $0, %eax
movq %rbp, %rsp
popq %rbp
ret
.data
.section .rodata
.bss
.balign 16
is_prime:
.zero 1000
.section .note.GNU-stack, "",@progbits
