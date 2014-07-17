SECTION_TEXT

GLOBAL example_sse2
HIDDEN example_sse2
FN example_sse2
	xorq %rax, %rax

	andq %rsi, %rsi
	jz Lexample_sse2_done

	testq $0xf, %rdi
	jz Lexample_sse2_aligned

Lexample_sse2_loop1:
	addb 0(%rdi), %al
	incq %rdi
	decq %rsi
	jz Lexample_sse2_done
	testq $0xf, %rdi
	jnz Lexample_sse2_loop1

Lexample_sse2_aligned:
	cmpq $32, %rsi
	jb Lexample_sse2_final31

	pxor %xmm0, %xmm0
	pxor %xmm1, %xmm1
Lexample_sse2_loop32:
	paddb 0(%rdi), %xmm0
	paddb 16(%rdi), %xmm1
	addq $32, %rdi
	subq $32, %rsi
	cmpq $32, %rsi
	jae Lexample_sse2_loop32

	paddb %xmm1, %xmm0
	pshufd $0xee, %xmm0, %xmm1
	paddb %xmm1, %xmm0
	pshufd $0x55, %xmm0, %xmm1
	paddb %xmm1, %xmm0
	movd %xmm0, %ecx
	addb %cl, %al
	shr $8, %ecx
	addb %cl, %al
	shr $8, %ecx
	addb %cl, %al
	shr $8, %ecx
	addb %cl, %al

Lexample_sse2_final31:
	andq %rsi, %rsi
	jz Lexample_sse2_done

Lexample_sse2_loop:
	addb 0(%rdi), %al
	incq %rdi
	decq %rsi
	jnz Lexample_sse2_loop

Lexample_sse2_done:
	ret
FN_END example_sse2
