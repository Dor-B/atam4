.global _start
.text
_start:
	movq $1, %rax
	movw $2, %bx
	movq $3, %rcx
        movq $0, %rdi
        Loop:
        cmpq $3, %rdi
        je Finish
	addq $1, %rax
	addw $1, %bx
	addq $1, %rcx
	incq %rdi
        jmp Loop; 
        Finish:
	movq $60, %rax
	xorq %rdi, %rdi
	syscall 
