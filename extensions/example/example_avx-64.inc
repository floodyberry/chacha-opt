SECTION_TEXT

GLOBAL example_avx
FN example_avx
	xorl %eax, %eax

	andq %rsi, %rsi
	jz Lexample_avx_done

	testq $0xf, %rdi
	jz Lexample_avx_aligned

Lexample_avx_loop1:
	addl 0(%rdi), %eax
	addq $4, %rdi
	decq %rsi
	jz Lexample_avx_done
	testq $0xf, %rdi
	jnz Lexample_avx_loop1

Lexample_avx_aligned:
	cmpq $8, %rsi
	jb Lexample_avx_final3

	vpxor %xmm0, %xmm0, %xmm0
	vpxor %xmm1, %xmm1, %xmm1
Lexample_avx_loop4:
	vpaddd 0(%rdi), %xmm0, %xmm0
	vpaddd 16(%rdi), %xmm1, %xmm1
	addq $32, %rdi
	subq $8, %rsi
	cmpq $8, %rsi
	jae Lexample_avx_loop4

	vpaddd %xmm1, %xmm0, %xmm0
	vpshufd $0xee, %xmm0, %xmm1
	vpaddd %xmm1, %xmm0, %xmm0
	vpshufd $0x55, %xmm0, %xmm1
	vpaddd %xmm1, %xmm0, %xmm0
	vmovd %xmm0, %ecx
	addl %ecx, %eax

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
