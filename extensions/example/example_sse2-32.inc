SECTION_TEXT

GLOBAL example_sse2
HIDDEN example_sse2
FN example_sse2
	movl 4(%esp), %edx
	movl 8(%esp), %ecx
	xorl %eax, %eax

	andl %ecx, %ecx
	jz Lexample_sse2_done

	testl $0xf, %edx
	jz Lexample_sse2_aligned

Lexample_sse2_loop1:
	addl 0(%edx), %eax
	addl $4, %edx
	decl %ecx
	jz Lexample_sse2_done
	testl $0xf, %edx
	jnz Lexample_sse2_loop1

Lexample_sse2_aligned:
	cmpl $8, %ecx
	jb Lexample_sse2_final3

	pushl %ebx
	pxor %xmm0, %xmm0
	pxor %xmm1, %xmm1
Lexample_sse2_loop4:
	paddd 0(%edx), %xmm0
	paddd 16(%edx), %xmm1
	addl $32, %edx
	subl $8, %ecx
	cmpl $8, %ecx
	jae Lexample_sse2_loop4

	paddd %xmm1, %xmm0
	pshufd $0xee, %xmm0, %xmm1
	paddd %xmm1, %xmm0
	pshufd $0x55, %xmm0, %xmm1
	paddd %xmm1, %xmm0
	movd %xmm0, %ebx
	addl %ebx, %eax
	popl %ebx

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
