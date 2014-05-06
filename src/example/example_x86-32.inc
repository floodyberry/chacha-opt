SECTION_TEXT

GLOBAL example_x86
FN example_x86
	movl 4(%esp), %edx
	movl 8(%esp), %ecx
	xorl %eax, %eax
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

