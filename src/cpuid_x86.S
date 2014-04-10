#if defined(__GNUC__)
#include "gcc_x86.inc"
#else
;.if 0
%include "yasm_x86.inc"
;.endif
#endif

SECTION_TEXT

/*
#define CPUID_MMX    (1 << 0)
#define CPUID_SSE    (1 << 1)
#define CPUID_SSE2   (1 << 2)
#define CPUID_SSE3   (1 << 3)
#define CPUID_SSSE3  (1 << 4)
#define CPUID_SSE4_1 (1 << 5)
#define CPUID_SSE4_2 (1 << 6)
#define CPUID_AVX    (1 << 7)
#define CPUID_XOP    (1 << 8)
#define CPUID_AVX2   (1 << 9)
#define CPUID_AVX512 (1 << 10)

#define CPUID_RDRAND    (1 << 26)
#define CPUID_POPCNT    (1 << 27)
#define CPUID_FMA4      (1 << 28)
#define CPUID_FMA3      (1 << 29)
#define CPUID_PCLMULQDQ (1 << 30)
#define CPUID_AES       (1 << 31)
*/

GLOBAL cpuid_x86
FN cpuid_x86
	CPUID_PROLOGUE

	/* use esi for flags */
	xorl %esi, %esi

	/* cpuid 0 */
	movl $0, %eax
	xorl %ecx, %ecx
	cpuid

	/* eax = max level, store in edi */
	movl %eax, %edi
	testl $0x00000500, %edi
	jz cpuid_x86_notp5

		/* Intel P5 pre-B0, only MMX */
		orl $(1 << 0), %esi
		jmp cpuid_x86_done

	cpuid_x86_notp5:

	/* cpuid 1 */
	movl $1, %eax
	xorl %ecx, %ecx
	cpuid

	/* rdrand */
	testl $(1 << 30), %ecx
	jz 1f
		orl $(1 << 26), %esi
	1:

	/* aes */
	testl $(1 << 25), %ecx
	jz 1f
		orl $(1 << 31), %esi
	1:

	/* popcnt */
	testl $(1 << 23), %ecx
	jz 1f
		orl $(1 << 27), %esi
	1:

	/* fma3 */
	testl $(1 << 12), %ecx
	jz 1f
		orl $(1 << 29), %esi
	1:

	/* pclmulqdq */
	testl $(1 << 1), %ecx
	jz 1f
		orl $(1 << 30), %esi
	1:

	/* SSE4.2 */
	testl $(1 << 20), %ecx
	jz 1f
		orl $(1 << 6), %esi
	1:

	/* SSE4.1 */
	testl $(1 << 19), %ecx
	jz 1f
		orl $(1 << 5), %esi
	1:

	/* SSSE3 */
	testl $(1 << 9), %ecx
	jz 1f
		orl $(1 << 4), %esi
	1:

	/* SSE3 */
	testl $(1     ), %ecx
	jz 1f
		orl $(1 << 3), %esi
	1:

	/* SSE2 */
	testl $(1 << 26), %edx
	jz 1f
		orl $(1 << 2), %esi
	1:

	/* SSE */
	testl $(1 << 25), %edx
	jz 1f
		orl $(1 << 1), %esi
	1:

	/* MMX */
	testl $(1 << 23), %ecx
	jz 1f
		orl $(1 << 0), %esi
	1:

	/* test for xsave enabled by os */
	testl $(1 << 27), %ecx
	jz cpuid_x86_skipavxplus

	/* test for avx supported by cpu */
	testl $(1 << 28), %ecx
	jz cpuid_x86_skipavxplus

	/* xgetbv(0) */
	xorl %ecx, %ecx
	.byte 0x0f, 0x01, 0xd0

	/* save XCR0 in scratch(ebp) */
	movl %eax, %ebp

	/* XCR0 & (XMM | YMM) */
	andl $((1 << 2) | (1 << 1)), %eax
	cmpl $((1 << 2) | (1 << 1)), %eax
	jne cpuid_x86_skipavxplus

		/* AVX is ok to use */
		orl $(1 << 7), %esi

		/* check for max level >= 7 */
		cmpl $7, %edi
		jb cpuid_x86_cpuid_below_7

			/* cpuid 7 */
			movl $7, %eax
			xorl %ecx, %ecx
			cpuid

			/* AVX2 */
			testl $(1 <<  5), %ebx
			jz 1f
				orl $(1 << 9), %esi
			1:

			/* XCR0 & (OPMASK | ZMMUPPER | ZMMEXTENDED) */
			andl $((1 << 5) | (1 << 6) | (1 << 7)), %ebp
			cmpl $((1 << 5) | (1 << 6) | (1 << 7)), %ebp
			jne cpuid_x86_skipavx512

				/* AVX-512 */
				testl $(1 << 16), %ebx
				jz 1f
					orl $(1 << 10), %esi
				1:

			cpuid_x86_skipavx512:

		cpuid_x86_cpuid_below_7:

		/* cpuid 0x80000000 */
		movl $0x80000000, %eax
		xorl %ecx, %ecx
		cpuid

		/* eax = max extended level  */
		cmpl $0x80000001, %eax
		jb cpuid_x86_skipxopplus

			/* cpuid $0x80000001 */
			movl $0x80000001, %eax
			xorl %ecx, %ecx
			cpuid

			/* fma4 */
			testl $(1 << 16), %ecx
			jz 1f
				orl $(1 << 28), %esi
			1:

			/* XOP */
			testl $(1 << 11), %ecx
			jz 1f
				orl $(1 << 8), %esi
			1:

	cpuid_x86_skipxopplus:

	cpuid_x86_skipavxplus:

cpuid_x86_done:
	movl %esi, %eax

	CPUID_EPILOGUE
FN_END cpuid_x86

