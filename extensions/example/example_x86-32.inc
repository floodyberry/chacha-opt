SECTION_TEXT

GLOBAL example_x86
HIDDEN example_x86
FN example_x86
	movl 4(%esp), %edx
	movl 8(%esp), %ecx
	xorl %eax, %eax

	andl %ecx, %ecx
	jz Lexample_x86_done

	cmpl $8, %ecx
	jb Lexample_x86_loop

	pushl %ebx
	pushl %esi
	pushl %edi

	movl %edx, %edi
	movl %ecx, %esi

	xorl %ebx, %ebx
	xorl %ecx, %ecx
	xorl %edx, %edx

Lexample_x86_loop4:
	movl 0(%edi), %ebx
	movl 4(%edi), %ecx
	addb %bl, %al
	addb %cl, %dl
	shr $8, %ebx
	shr $8, %ecx
	addb %bl, %al
	addb %cl, %dl
	shr $8, %ebx
	shr $8, %ecx
	addb %bl, %al
	addb %cl, %dl
	shr $8, %ebx
	shr $8, %ecx
	addb %bl, %al
	addb %cl, %dl
	addl $8, %edi
	subl $8, %esi
	cmpl $8, %esi
	jae Lexample_x86_loop4

	addb %dl, %al

	movl %edi, %edx
	movl %esi, %ecx

	popl %edi
	popl %esi
	popl %ebx

	andl %ecx, %ecx
	jz Lexample_x86_done

Lexample_x86_loop:
	addb 0(%edx), %al
	incl %edx
	decl %ecx
	jnz Lexample_x86_loop

Lexample_x86_done:
	andl $0xff, %eax
	ret
FN_END example_x86

