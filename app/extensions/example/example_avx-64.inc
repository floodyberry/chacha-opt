SECTION_TEXT

GLOBAL example_avx
HIDDEN example_avx
FN example_avx
	xorl %eax, %eax

	andq %rsi, %rsi
	jz Lexample_avx_done

	testq $0xf, %rdi
	jz Lexample_avx_aligned

Lexample_avx_loop1:
	addb 0(%rdi), %al
	incq %rdi
	decq %rsi
	jz Lexample_avx_done
	testq $0xf, %rdi
	jnz Lexample_avx_loop1

Lexample_avx_aligned:
	cmpq $32, %rsi
	jb Lexample_avx_final3

	vpxor %xmm0, %xmm0, %xmm0
	vpxor %xmm1, %xmm1, %xmm1
Lexample_avx_loop4:
	vpaddb 0(%rdi), %xmm0, %xmm0
	vpaddb 16(%rdi), %xmm1, %xmm1
	addq $32, %rdi
	subq $32, %rsi
	cmpq $32, %rsi
	jae Lexample_avx_loop4

	vpaddb %xmm1, %xmm0, %xmm0
	vpshufd $0xee, %xmm0, %xmm1
	vpaddb %xmm1, %xmm0, %xmm0
	vpshufd $0x55, %xmm0, %xmm1
	vpaddb %xmm1, %xmm0, %xmm0
	vmovd %xmm0, %ecx
	addb %cl, %al
	shr $8, %ecx
	addb %cl, %al
	shr $8, %ecx
	addb %cl, %al
	shr $8, %ecx
	addb %cl, %al

Lexample_avx_final3:
	andq %rsi, %rsi
	jz Lexample_avx_done

Lexample_avx_loop:
	addb 0(%rdi), %al
	incq %rdi
	decq %rsi
	jnz Lexample_avx_loop

Lexample_avx_done:
	ret
FN_END example_avx
