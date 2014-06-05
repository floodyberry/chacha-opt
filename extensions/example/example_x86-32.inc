SECTION_TEXT

GLOBAL example_x86
FN example_x86
	movl 4(%esp), %edx
	movl 8(%esp), %ecx
	xorl %eax, %eax

	andl %ecx, %ecx
	jz Lexample_x86_done

	cmpl $4, %ecx
	jb Lexample_x86_loop

	pushl %ebx
	pushl %esi
	pushl %edi

	xorl %ebx, %ebx
	xorl %esi, %esi
	xorl %edi, %edi

Lexample_x86_loop4:
	addl 0(%edx), %eax
	addl 4(%edx), %ebx
	addl 8(%edx), %esi
	addl 12(%edx), %edi
	addl $16, %edx
	subl $4, %ecx
	cmpl $4, %ecx
	jae Lexample_x86_loop4

	addl %ebx, %eax
	addl %edi, %esi
	addl %esi, %eax

	popl %edi
	popl %esi
	popl %ebx

	andl %ecx, %ecx
	jz Lexample_x86_done

Lexample_x86_loop:
	addl 0(%edx), %eax
	addl $4, %edx
	decl %ecx
	jnz Lexample_x86_loop

Lexample_x86_done:
	ret
FN_END example_x86

