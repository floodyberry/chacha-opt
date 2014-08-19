SECTION_TEXT

GLOBAL_HIDDEN_FN example_sse2
	movl 4(%esp), %edx
	movl 8(%esp), %ecx
	xorl %eax, %eax

	andl %ecx, %ecx
	jz Lexample_sse2_done

	testl $0xf, %edx
	jz Lexample_sse2_aligned

Lexample_sse2_loop1:
	addb 0(%edx), %al
	incl %edx
	decl %ecx
	jz Lexample_sse2_done
	testl $0xf, %edx
	jnz Lexample_sse2_loop1

Lexample_sse2_aligned:
	cmpl $32, %ecx
	jb Lexample_sse2_final31

	pushl %ebx
	pxor %xmm0, %xmm0
	pxor %xmm1, %xmm1
Lexample_sse2_loop32:
	paddb 0(%edx), %xmm0
	paddb 16(%edx), %xmm1
	addl $32, %edx
	subl $32, %ecx
	cmpl $32, %ecx
	jae Lexample_sse2_loop32

	paddb %xmm1, %xmm0
	pshufd $0xee, %xmm0, %xmm1
	paddb %xmm1, %xmm0
	pshufd $0x55, %xmm0, %xmm1
	paddb %xmm1, %xmm0
	movd %xmm0, %ebx
	addb %bl, %al
	shr $8, %ebx
	addb %bl, %al
	shr $8, %ebx
	addb %bl, %al
	shr $8, %ebx
	addb %bl, %al
	popl %ebx

Lexample_sse2_final31:
	andl %ecx, %ecx
	jz Lexample_sse2_done

Lexample_sse2_loop:
	addb 0(%edx), %al
	incl %edx
	decl %ecx
	jnz Lexample_sse2_loop

Lexample_sse2_done:
	andb $0xff, %al
	ret
FN_END example_sse2
