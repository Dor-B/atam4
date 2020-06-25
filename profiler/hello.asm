.global _start
.text
_start:
	movq $1, %rax
	movw $2, %bx
	movq $3, %rcx
	nop
	movq $4, %rax
	movw $5, %bx
	movq $6, %rcx
	nop
	movq $60, %rax
	xorq %rdi, %rdi
	syscall 
