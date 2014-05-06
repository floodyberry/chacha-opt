SECTION_TEXT

GLOBAL example_avx
FN example_avx
	xorl %eax, %eax

	cmpq $4, %rsi
	jb Lexample_avx_final3

	vpxor %xmm0, %xmm0, %xmm0
Lexample_avx_loop4:
	vmovdqu 0(%rdi), %xmm1
	vpaddd %xmm1, %xmm0, %xmm0
	addq $16, %rdi
	subq $4, %rsi
	cmpq $4, %rsi
	jae Lexample_avx_loop4

	vpshufd $0xee, %xmm0, %xmm1
	vpaddd %xmm1, %xmm0, %xmm0
	vpshufd $0x55, %xmm0, %xmm1
	vpaddd %xmm1, %xmm0, %xmm0
	vmovd %xmm0, %eax

Lexample_avx_final3:
	andq %rsi, %rsi
	jz Lexample_avx_done

Lexample_avx_loop:
	addl 0(%rdi), %eax
	addq $4, %rdi
	decq %rsi
	jnz Lexample_avx_loop

Lexample_avx_done:
	ret
FN_END example_avx
