#include <stdio.h>
#include "config.h"

#if defined(ARCH_X86)
extern unsigned long cpuid_x86();

enum cpuid_flags_t {
	CPUID_MMX       = (1 <<  0),
	CPUID_SSE       = (1 <<  1),
	CPUID_SSE2      = (1 <<  2),
	CPUID_SSE3      = (1 <<  3),
	CPUID_SSSE3     = (1 <<  4),
	CPUID_SSE4_1    = (1 <<  5),
	CPUID_SSE4_2    = (1 <<  6),
	CPUID_AVX       = (1 <<  7),
	CPUID_XOP       = (1 <<  8),
	CPUID_AVX2      = (1 <<  9),
	CPUID_AVX512    = (1 << 10),

	CPUID_RDRAND    = (1 << 26),
	CPUID_POPCNT    = (1 << 27),
	CPUID_FMA4      = (1 << 28),
	CPUID_FMA3      = (1 << 29),
	CPUID_PCLMULQDQ = (1 << 30),
	CPUID_AES       = (1 << 31)
};

static void
print_cpuflags(void) {
	unsigned long cpuflags = cpuid_x86();

	printf("cpu extensions:\n");
	if (cpuflags & CPUID_MMX)
		printf("  mmx\n");
	if (cpuflags & CPUID_SSE) {
		printf("  sse");
		if (cpuflags & CPUID_SSE2)
			printf(",sse2");
		if (cpuflags & CPUID_SSE3)
			printf(",sse3");
		if (cpuflags & CPUID_SSSE3)
			printf(",ssse3");
		if (cpuflags & CPUID_SSE4_1)
			printf(",sse4.1");
		if (cpuflags & CPUID_SSE4_2)
			printf(",sse4.2");
		printf("\n");
	}
	if (cpuflags & CPUID_AVX) {
		printf("  avx");
		if (cpuflags & CPUID_AVX2)
			printf(",avx2");
		if (cpuflags & CPUID_AVX512)
			printf(",avx512");
		printf("\n");
	}
	if (cpuflags & CPUID_XOP)
		printf("  xop\n");

	printf("cpu features:\n ");
	if (cpuflags & CPUID_RDRAND)
		printf(" rdrand");
	if (cpuflags & CPUID_POPCNT)
		printf(" popcnt");
	if (cpuflags & CPUID_FMA4)
		printf(" fma4");
	if (cpuflags & CPUID_FMA3)
		printf(" fma3");
	if (cpuflags & CPUID_PCLMULQDQ)
		printf(" pclmulqdq");
	if (cpuflags & CPUID_AES)
		printf(" aes");
	printf("\n");
}
#endif /* defined(ARCH_X86) */

int main(void) {
	print_cpuflags();
	return 0;
}
