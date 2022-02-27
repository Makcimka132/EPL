.extern main

.global _EPLf4exit
.type	_EPLf4exit, @function
_EPLf4exit:
	movl $1, %eax
	int $0x80

.global write
write:
	push ebp
	movl %esp, %ebp

	movl $4, %eax
	movl $1, %ebx
	movl 8(%ebp), %ecx
	movl 12(%ebp), %edx

	int $0x80

	pop ebp
	ret

.global _start
.type	_start, @function
_start:
	call main
	
	push $1
	push $0x21
	call write

   	movl %eax, %ebx
	call exit