SECTION_TEXT

GLOBAL example_sse2
FN example_sse2
	movl 4(%esp), %edx
	movl 8(%esp), %ecx
	xorl %eax, %eax

	cmpl $4, %ecx
	jb Lexample_sse2_final3

	pxor %xmm0, %xmm0
Lexample_sse2_loop4:
	movdqu 0(%edx), %xmm1
	paddd %xmm1, %xmm0
	addl $16, %edx
	subl $4, %ecx
	cmpl $4, %ecx
	jae Lexample_sse2_loop4

	pshufd $0xee, %xmm0, %xmm1
	paddd %xmm1, %xmm0
	pshufd $0x55, %xmm0, %xmm1
	paddd %xmm1, %xmm0
	movd %xmm0, %eax

Lexample_sse2_final3:
	andl %ecx, %ecx
	jz Lexample_sse2_done

Lexample_sse2_loop:
	addl 0(%edx), %eax
	addl $4, %edx
	decl %ecx
	jnz Lexample_sse2_loop

Lexample_sse2_done:
	ret
FN_END example_sse2
