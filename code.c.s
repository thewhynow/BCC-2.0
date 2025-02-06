.text
.globl alloc_array
alloc_array:
pushq %rbp
movq %rsp, %rbp
subq $80, %rsp
movq %rdi, -8(%rbp)
movq %rsi, -16(%rbp)
pushq %rdi
pushq %rsi
pushq %rdx
pushq %rcx
pushq %r8
pushq %r9
pushq %r10
pushq %r11
subq $0, %rsp
movq -16(%rbp), %r10
imulq -8(%rbp), %r10
movq %r10, -40(%rbp)
movq $24, -48(%rbp)
movq -40(%rbp), %r10
addq %r10, -48(%rbp)
movq -48(%rbp), %rdi
call malloc
addq $0, %rsp
pop %r11
pop %r10
pop %r9
pop %r8
pop %rcx
pop %rdx
pop %rsi
pop %rdi
movq %rax, -24(%rbp)
movq -24(%rbp), %rax
movq %rax, -32(%rbp)
movq -32(%rbp), %r11
movq -16(%rbp), %r10
movq %r10, (%r11)
movq -32(%rbp), %rax
movq $1, %rdx
leaq (%rax, %rdx, 8), %rax
movq %rax, -56(%rbp)
movq -56(%rbp), %r11
movq $0, (%r11)
movq -32(%rbp), %rax
movq $2, %rdx
leaq (%rax, %rdx, 8), %rax
movq %rax, -64(%rbp)
movq -64(%rbp), %r11
movq -8(%rbp), %r10
movq %r10, (%r11)
movq -32(%rbp), %rax
movq $3, %rdx
leaq (%rax, %rdx, 8), %rax
movq %rax, -72(%rbp)
movq -72(%rbp), %rax
movq %rbp, %rsp
popq %rbp
ret
movl $0, %eax
movq %rbp, %rsp
popq %rbp
ret
.globl free_array
free_array:
pushq %rbp
movq %rsp, %rbp
subq $112, %rsp
movq %rdi, -8(%rbp)
movq %rsi, -16(%rbp)
cmpq $0, -16(%rbp)
je L0
movq -8(%rbp), %rax
movq $2, %rdx
negq %rdx
leaq (%rax, %rdx, 8), %rax
movq %rax, -40(%rbp)
movq -40(%rbp), %rax
movq %rax, -24(%rbp)
movq -24(%rbp), %rax
movq $1, %rdx
leaq (%rax, %rdx, 8), %rax
movq %rax, -48(%rbp)
movq -48(%rbp), %rax
movq %rax, -32(%rbp)
movq $0, -56(%rbp)
L4:
movq -24(%rbp), %r11
movq (%r11), %rax
movq %rax, -64(%rbp)
movq -64(%rbp), %r10
cmpq %r10, -56(%rbp)
movl $0, -68(%rbp)
setb -68(%rbp)
cmpl $0, -68(%rbp)
je L3
pushq %rdi
pushq %rsi
pushq %rdx
pushq %rcx
pushq %r8
pushq %r9
pushq %r10
pushq %r11
subq $0, %rsp
movq -32(%rbp), %r11
movq (%r11), %rax
movq %rax, -76(%rbp)
movq -56(%rbp), %r10
imulq -76(%rbp), %r10
movq %r10, -84(%rbp)
movq -8(%rbp), %rax
movq -84(%rbp), %rdx
leaq (%rax, %rdx, 1), %rax
movq %rax, -92(%rbp)
movq -92(%rbp), %rdi
call *-16(%rbp)
addq $0, %rsp
pop %r11
pop %r10
pop %r9
pop %r8
pop %rcx
pop %rdx
pop %rsi
pop %rdi
L2:
incq -56(%rbp)
jmp L4
L3:
jmp L1
L0:
L1:
pushq %rdi
pushq %rsi
pushq %rdx
pushq %rcx
pushq %r8
pushq %r9
pushq %r10
pushq %r11
subq $0, %rsp
movq -8(%rbp), %rax
movq $3, %rdx
negq %rdx
leaq (%rax, %rdx, 8), %rax
movq %rax, -100(%rbp)
movq -100(%rbp), %rdi
call free
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
.globl pback_array
pback_array:
pushq %rbp
movq %rsp, %rbp
subq $256, %rsp
movq %rdi, -8(%rbp)
movq %rsi, -16(%rbp)
movq -8(%rbp), %r11
movq (%r11), %rax
movq %rax, -56(%rbp)
movq -56(%rbp), %rax
movq $1, %rdx
negq %rdx
leaq (%rax, %rdx, 8), %rax
movq %rax, -64(%rbp)
movq -64(%rbp), %r11
movq (%r11), %rax
movq %rax, -72(%rbp)
movq -72(%rbp), %rax
movq %rax, -24(%rbp)
movq -8(%rbp), %r11
movq (%r11), %rax
movq %rax, -80(%rbp)
movq -80(%rbp), %rax
movq $2, %rdx
negq %rdx
leaq (%rax, %rdx, 8), %rax
movq %rax, -88(%rbp)
movq -88(%rbp), %rax
movq %rax, -32(%rbp)
movq -8(%rbp), %r11
movq (%r11), %rax
movq %rax, -96(%rbp)
movq -96(%rbp), %rax
movq $3, %rdx
negq %rdx
leaq (%rax, %rdx, 8), %rax
movq %rax, -104(%rbp)
movq -104(%rbp), %rax
movq %rax, -40(%rbp)
movq -32(%rbp), %r11
movq (%r11), %rax
movq %rax, -112(%rbp)
movq -40(%rbp), %r11
movq (%r11), %rax
movq %rax, -120(%rbp)
movq -120(%rbp), %r10
cmpq %r10, -112(%rbp)
movl $0, -124(%rbp)
sete -124(%rbp)
cmpl $0, -124(%rbp)
je L5
movq -40(%rbp), %r11
movq (%r11), %rax
movq %rax, -132(%rbp)
movq -40(%rbp), %r11
movq (%r11), %rax
movq %rax, -140(%rbp)
movq -140(%rbp), %rax
movq $0, %rdx
movq $2, %r10
divq %r10
movq %rax, -148(%rbp)
movq -148(%rbp), %rax
movq %rax, -156(%rbp)
addq $1, -156(%rbp)
movq -132(%rbp), %rax
movq %rax, -164(%rbp)
movq -156(%rbp), %r10
addq %r10, -164(%rbp)
movq -40(%rbp), %r11
movq -164(%rbp), %r10
movq %r10, (%r11)
pushq %rdi
pushq %rsi
pushq %rdx
pushq %rcx
pushq %r8
pushq %r9
pushq %r10
pushq %r11
subq $0, %rsp
movq -40(%rbp), %rdi
movq -40(%rbp), %r11
movq (%r11), %rax
movq %rax, -172(%rbp)
movq -172(%rbp), %r10
imulq -24(%rbp), %r10
movq %r10, -180(%rbp)
movq $24, -188(%rbp)
movq -180(%rbp), %r10
addq %r10, -188(%rbp)
movq -188(%rbp), %rsi
call realloc
addq $0, %rsp
pop %r11
pop %r10
pop %r9
pop %r8
pop %rcx
pop %rdx
pop %rsi
pop %rdi
movq %rax, %rax
movq $24, %rdx
leaq (%rax, %rdx, 1), %rax
movq %rax, -196(%rbp)
movq -8(%rbp), %r11
movq -196(%rbp), %r10
movq %r10, (%r11)
movq -8(%rbp), %r11
movq (%r11), %rax
movq %rax, -204(%rbp)
cmpq $0, -204(%rbp)
je L7
movq -8(%rbp), %r11
movq (%r11), %rax
movq %rax, -212(%rbp)
movq -212(%rbp), %rax
movq $2, %rdx
negq %rdx
leaq (%rax, %rdx, 8), %rax
movq %rax, -220(%rbp)
movq -220(%rbp), %rax
movq %rax, -32(%rbp)
jmp L8
L7:
pushq %rdi
pushq %rsi
pushq %rdx
pushq %rcx
pushq %r8
pushq %r9
pushq %r10
pushq %r11
subq $0, %rsp
leaq .string_0(%rip), %r11
movq %r11, %rdi
call perror
addq $0, %rsp
pop %r11
pop %r10
pop %r9
pop %r8
pop %rcx
pop %rdx
pop %rsi
pop %rdi
L8:
jmp L6
L5:
L6:
movq -8(%rbp), %r11
movq (%r11), %rax
movq %rax, -228(%rbp)
movq -32(%rbp), %r11
movq (%r11), %rax
movq %rax, -236(%rbp)
incq (%r11)
movq -24(%rbp), %r10
imulq -236(%rbp), %r10
movq %r10, -244(%rbp)
movq -228(%rbp), %rax
movq -244(%rbp), %rdx
leaq (%rax, %rdx, 1), %rax
movq %rax, -252(%rbp)
movq -252(%rbp), %rax
movq %rax, -48(%rbp)
pushq %rdi
pushq %rsi
pushq %rdx
pushq %rcx
pushq %r8
pushq %r9
pushq %r10
pushq %r11
subq $0, %rsp
movq -48(%rbp), %rdi
movq -16(%rbp), %rsi
movq -24(%rbp), %rdx
call memcpy
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
.globl main
main:
pushq %rbp
movq %rsp, %rbp
subq $48, %rsp
pushq %rdi
pushq %rsi
pushq %rdx
pushq %rcx
pushq %r8
pushq %r9
pushq %r10
pushq %r11
subq $0, %rsp
movq $4, %rdi
movq $1, %rsi
call alloc_array
addq $0, %rsp
pop %r11
pop %r10
pop %r9
pop %r8
pop %rcx
pop %rdx
pop %rsi
pop %rdi
movq %rax, -8(%rbp)
movl $0, -12(%rbp)
L11:
cmpl $10, -12(%rbp)
movl $0, -16(%rbp)
setl -16(%rbp)
cmpl $0, -16(%rbp)
je L10
pushq %rdi
pushq %rsi
pushq %rdx
pushq %rcx
pushq %r8
pushq %r9
pushq %r10
pushq %r11
subq $0, %rsp
leaq -8(%rbp), %r11
movq %r11, %rdi
leaq -12(%rbp), %r11
movq %r11, %rsi
call pback_array
addq $0, %rsp
pop %r11
pop %r10
pop %r9
pop %r8
pop %rcx
pop %rdx
pop %rsi
pop %rdi
L9:
incl -12(%rbp)
jmp L11
L10:
movq $0, -24(%rbp)
L14:
cmpq $10, -24(%rbp)
movl $0, -28(%rbp)
setb -28(%rbp)
cmpl $0, -28(%rbp)
je L13
pushq %rdi
pushq %rsi
pushq %rdx
pushq %rcx
pushq %r8
pushq %r9
pushq %r10
pushq %r11
subq $0, %rsp
leaq .string_1(%rip), %r11
movq %r11, %rdi
movq -8(%rbp), %rax
movq -24(%rbp), %rdx
leaq (%rax, %rdx, 4), %rax
movq %rax, -36(%rbp)
movq -36(%rbp), %r11
movq (%r11), %rax
movq %rax, -40(%rbp)
movl -40(%rbp), %esi
call printf
addq $0, %rsp
pop %r11
pop %r10
pop %r9
pop %r8
pop %rcx
pop %rdx
pop %rsi
pop %rdi
L12:
incq -24(%rbp)
jmp L14
L13:
pushq %rdi
pushq %rsi
pushq %rdx
pushq %rcx
pushq %r8
pushq %r9
pushq %r10
pushq %r11
subq $0, %rsp
movq -8(%rbp), %rdi
movq $0, %rsi
call free_array
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
movl $10, %edi
call putchar
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
.section .rodata
.string_0:
.asciz "ran out of memory while allocating array"
.string_1:
.asciz "%i "
.bss
.section .note.GNU-stack, "",@progbits
