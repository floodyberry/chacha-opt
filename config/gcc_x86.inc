#ifndef BASE_GCC_S
#define BASE_GCC_S

#include "config.h"

#if !defined(HAVE_SLASHMACRO) && !defined(HAVE_DOLLARMACRO)
	#error Unknown gnu as macro parameter convention! Run ./configure
#endif

#define IS_ELF (defined(__ELF__))
#define IS_MACH (defined(__MACH__))
#define IS_BITS64 (defined(__x86_64__))
#define IS_BITS32 (defined(__i386__  ))
#define IS_WIN32 (IS_BITS32 && (defined(_WIN32) || defined(__CYGWIN__)))
#define IS_WIN64 (IS_BITS64 && (defined(_WIN64) || defined(__CYGWIN__)))



#if (IS_WIN64)
	/* handles 0-6 arguments and optional saving of the upper 8 xmm registers */
	.macro WIN64STUBFN name, args, xmmused
		\name:; _\name:;

		subq $(184), %rsp
		movdqa %xmm6, 0(%rsp)
		movdqa %xmm7, 16(%rsp)
		.if \xmmused > 8
			movdqa %xmm8, 32(%rsp)
			movdqa %xmm9, 48(%rsp)
			movdqa %xmm10, 64(%rsp)
			movdqa %xmm11, 80(%rsp)
			movdqa %xmm12, 96(%rsp)
			movdqa %xmm13, 112(%rsp)
			movdqa %xmm14, 128(%rsp)
			movdqa %xmm15, 144(%rsp)
		.endif
		movq %rdi, 160(%rsp)
		movq %rsi, 168(%rsp)
		movq %rcx, %rdi
		movq %rdx, %rsi
		movq %r8, %rdx
		movq %r9, %rcx
		.if \args >= 5
			movq 224(%rsp), %r8
		.endif
		.if \args >= 6
			movq 232(%rsp), %r9
		.endif
		call thunk_\name
		movdqa 0(%rsp), %xmm6
		movdqa 16(%rsp), %xmm7
		.if \xmmused > 8
			movdqa 32(%rsp), %xmm8
			movdqa 48(%rsp), %xmm9
			movdqa 64(%rsp), %xmm10
			movdqa 80(%rsp), %xmm11
			movdqa 96(%rsp), %xmm12
			movdqa 112(%rsp), %xmm13
			movdqa 128(%rsp), %xmm14
			movdqa 144(%rsp), %xmm15
		.endif
		movq 160(%rsp), %rdi
		movq 168(%rsp), %rsi
		addq $(184), %rsp
		ret
	thunk_\name:
	.endm

	.macro FN name
		WIN64STUBFN \name, 4, 16
	.endm

	.macro FN_EXT name, args, xmmused
		WIN64STUBFN \name, \args, \xmmused
	.endm

	.macro FN_END name
	.endm
#elif (IS_WIN32)
	.macro FN name
		\name:
		_\name:
	.endm

	.macro FN_EXT name, args, xmmused
		FN \name
	.endm

	.macro FN_END name
	.endm
#elif (IS_ELF)
	.macro FN name
		\name:
		_\name:
	.endm

	.macro FN_EXT name, args, xmmused
		FN \name
	.endm

	.macro FN_END name
		.size \name, .-\name
		.size _\name, .-_\name
		.type \name, @function
	 	.type _\name, @function
	.endm

	/* set NX for stack */
	.section .note.GNU-stack,"",@progbits
#elif (IS_MACH)
	.macro FN name
	#if defined(HAVE_SLASHMACRO)
		\name:
		_\name:
	#elif defined(HAVE_DOLLARMACRO)
		$0:
		_$0:
	#endif
	.endm

	.macro FN_EXT name, args, xmmused
	#if defined(HAVE_SLASHMACRO)
		FN \name
	#elif defined(HAVE_DOLLARMACRO)
		FN $0
	#endif
	.endm

	.macro FN_END name
	.endm
#endif

/* put everything in the code segment to simplify things */
.macro SECTION_TEXT
	.text
.endm

.macro SECTION_RODATA
	.text
.endm

/* declare a global function */
.macro GLOBAL name
#if defined(HAVE_SLASHMACRO)
	.globl \name
	.globl _\name
#elif defined(HAVE_DOLLARMACRO)
	.globl $0
	.globl _$0
#endif
.endm

/* pic support */
.macro LOAD_VAR_PIC var, reg
#if (IS_BITS32)
	#if defined(HAVE_SLASHMACRO)
		call 1f
		1:
		popl \reg
		leal \var - 1b(\reg), \reg
	#elif defined(HAVE_DOLLARMACRO)
		call 1f
		1:
		popl $1
		leal $0 - 1b($1), $1
	#endif
#else
	#if defined(HAVE_SLASHMACRO)
		leaq \var(%rip), \reg
	#elif defined(HAVE_DOLLARMACRO)
		leaq $0(%rip), $1
	#endif
#endif
.endm

#if defined(HAVE_SLASHMACRO)
	#define INCLUDE_FILE_PARM "\file"
#elif defined(HAVE_DOLLARMACRO)
	#define INCLUDE_FILE_PARM "$0"
#endif


.macro INCLUDE file
	.include  INCLUDE_FILE_PARM
.endm

/* include the file with the variable(s) if variable 'name' is not already included */
.macro INCLUDE_VAR_FILE file, name
	#if defined(HAVE_SLASHMACRO)
		.ifndef \name
			.include INCLUDE_FILE_PARM
		.endif
	#elif defined(HAVE_DOLLARMACRO)
		.ifndef $0
			.include INCLUDE_FILE_PARM
		.endif
	#endif
.endm

.macro INCLUDE_IF_32BIT file
#if (IS_BITS32)
	.include INCLUDE_FILE_PARM
#endif
.endm

.macro INCLUDE_IF_64BIT file
#if (IS_BITS64)
	.include INCLUDE_FILE_PARM
#endif
.endm


.macro INCLUDE_IF_MMX_32BIT file
#if defined(HAVE_MMX)
	INCLUDE_IF_32BIT INCLUDE_FILE_PARM
#endif
.endm

.macro INCLUDE_IF_MMX_64BIT file
#if defined(HAVE_MMX)
	INCLUDE_IF_64BIT INCLUDE_FILE_PARM
#endif
.endm


.macro INCLUDE_IF_SSE_32BIT file
#if defined(HAVE_SSE)
	INCLUDE_IF_32BIT INCLUDE_FILE_PARM
#endif
.endm

.macro INCLUDE_IF_SSE_64BIT file
#if defined(HAVE_SSE)
	INCLUDE_IF_64BIT INCLUDE_FILE_PARM
#endif
.endm


.macro INCLUDE_IF_SSE2_32BIT file
#if defined(HAVE_SSE2)
	INCLUDE_IF_32BIT INCLUDE_FILE_PARM
#endif
.endm

.macro INCLUDE_IF_SSE2_64BIT file
#if defined(HAVE_SSE2)
	INCLUDE_IF_64BIT INCLUDE_FILE_PARM
#endif
.endm


.macro INCLUDE_IF_SSE3_32BIT file
#if defined(HAVE_SSE3)
	INCLUDE_IF_32BIT INCLUDE_FILE_PARM
#endif
.endm

.macro INCLUDE_IF_SSE3_64BIT file
#if defined(HAVE_SSE3)
	INCLUDE_IF_64BIT INCLUDE_FILE_PARM
#endif
.endm


.macro INCLUDE_IF_SSSE3_32BIT file
#if defined(HAVE_SSSE3)
	INCLUDE_IF_32BIT INCLUDE_FILE_PARM
#endif
.endm

.macro INCLUDE_IF_SSSE3_64BIT file
#if defined(HAVE_SSSE3)
	INCLUDE_IF_64BIT INCLUDE_FILE_PARM
#endif
.endm


.macro INCLUDE_IF_SSE4_1_32BIT file
#if defined(HAVE_SSE4_1)
	INCLUDE_IF_32BIT INCLUDE_FILE_PARM
#endif
.endm

.macro INCLUDE_IF_SSE4_1_64BIT file
#if defined(HAVE_SSE4_1)
	INCLUDE_IF_64BIT INCLUDE_FILE_PARM
#endif
.endm


.macro INCLUDE_IF_SSE4_2_32BIT file
#if defined(HAVE_SSE4_2)
	INCLUDE_IF_32BIT INCLUDE_FILE_PARM
#endif
.endm

.macro INCLUDE_IF_SSE4_2_64BIT file
#if defined(HAVE_SSE4_2)
	INCLUDE_IF_64BIT INCLUDE_FILE_PARM
#endif
.endm


.macro INCLUDE_IF_AVX_32BIT file
#if defined(HAVE_AVX)
	INCLUDE_IF_32BIT INCLUDE_FILE_PARM
#endif
.endm

.macro INCLUDE_IF_AVX_64BIT file
#if defined(HAVE_AVX)
	INCLUDE_IF_64BIT INCLUDE_FILE_PARM
#endif
.endm


.macro INCLUDE_IF_XOP_32BIT file
#if defined(HAVE_XOP)
	INCLUDE_IF_32BIT INCLUDE_FILE_PARM
#endif
.endm

.macro INCLUDE_IF_XOP_64BIT file
#if defined(HAVE_XOP)
	INCLUDE_IF_64BIT INCLUDE_FILE_PARM
#endif
.endm


.macro INCLUDE_IF_AVX2_32BIT file
#if defined(HAVE_AVX2)
	INCLUDE_IF_32BIT INCLUDE_FILE_PARM
#endif
.endm

.macro INCLUDE_IF_AVX2_64BIT file
#if defined(HAVE_AVX2)
	INCLUDE_IF_64BIT INCLUDE_FILE_PARM
#endif
.endm


.macro INCLUDE_IF_AVX512_32BIT file
#if defined(HAVE_AVX512)
	INCLUDE_IF_32BIT INCLUDE_FILE_PARM
#endif
.endm

.macro INCLUDE_IF_AVX512_64BIT file
#if defined(HAVE_AVX512)
	INCLUDE_IF_64BIT INCLUDE_FILE_PARM
#endif
.endm


/* stupid helpers so we can have cpuid in one file */

.macro CPUID_PROLOGUE
#if (IS_BITS32)
	pushl %ebx
	pushl %esi
	pushl %edi
	pushl %ebp

	/* check that cpuid is supported */
	pushfl
	popl %eax
	movl %eax, %ecx
#if defined(HAVE_SLASHMACRO)
	xorl $(0x200000), %eax
#elif defined(HAVE_DOLLARMACRO)
	xorl $$0x200000, %eax
#endif
	pushl %eax
	popfl
	pushfl
	popl %eax
	xorl %ecx, %eax
#if defined(HAVE_SLASHMACRO)
	shrl $(21), %eax
	andl $(1), %eax
#elif defined(HAVE_DOLLARMACRO)
	shrl $$21, %eax
	andl $$1, %eax
#endif
	pushl %ecx
	popfl
	andl %eax, %eax
	jz cpuid_x86_done
#else
	pushq %rbx
	pushq %rsi
	pushq %rdi
	pushq %rbp
#endif
.endm

.macro CPUID_EPILOGUE
#if (IS_BITS32)
	popl %ebp
	popl %edi
	popl %esi
	popl %ebx
#else
	popq %rbp
	popq %rdi
	popq %rsi
	popq %rbx
#endif
	ret
.endm


#endif /* BASE_GCC_S */
