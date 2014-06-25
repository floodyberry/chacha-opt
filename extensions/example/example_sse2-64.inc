SECTION_TEXT

GLOBAL example_sse2
HIDDEN example_sse2
FN example_sse2
	xorl %eax, %eax

	andq %rsi, %rsi
	jz Lexample_sse2_done

	testq $0xf, %rdi
	jz Lexample_sse2_aligned

Lexample_sse2_loop1:
	addl 0(%rdi), %eax
	addq $4, %rdi
	decq %rsi
	jz Lexample_sse2_done
	testq $0xf, %rdi
	jnz Lexample_sse2_loop1

Lexample_sse2_aligned:
	cmpq $8, %rsi
	jb Lexample_sse2_final3

	pxor %xmm0, %xmm0
	pxor %xmm1, %xmm1
Lexample_sse2_loop4:
	paddd 0(%rdi), %xmm0
	paddd 16(%rdi), %xmm1
	addq $32, %rdi
	subq $8, %rsi
	cmpq $8, %rsi
	jae Lexample_sse2_loop4

	paddd %xmm1, %xmm0
	pshufd $0xee, %xmm0, %xmm1
	paddd %xmm1, %xmm0
	pshufd $0x55, %xmm0, %xmm1
	paddd %xmm1, %xmm0
	movd %xmm0, %ecx
	addl %ecx, %eax

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
