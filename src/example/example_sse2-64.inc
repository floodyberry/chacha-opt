SECTION_TEXT

GLOBAL example_sse2
FN example_sse2
	xorl %eax, %eax

	cmpq $4, %rsi
	jb Lexample_sse2_final3

	pxor %xmm0, %xmm0
Lexample_sse2_loop4:
	movdqu 0(%rdi), %xmm1
	paddd %xmm1, %xmm0
	addq $16, %rdi
	subq $4, %rsi
	cmpq $4, %rsi
	jae Lexample_sse2_loop4

	pshufd $0xee, %xmm0, %xmm1
	paddd %xmm1, %xmm0
	pshufd $0x55, %xmm0, %xmm1
	paddd %xmm1, %xmm0
	movd %xmm0, %eax

Lexample_sse2_final3:
	andq %rsi, %rsi
	jz Lexample_sse2_done

Lexample_sse2_loop:
	addl 0(%rdi), %eax
	addq $4, %rdi
	decq %rsi
	jnz Lexample_sse2_loop

Lexample_sse2_done:
	ret
FN_END example_sse2
