#include <stdio.h>
#include <string.h>
#include "cpuid.h"
#include "example.h"

#if defined(ARCH_X86)
static void
print_cpuflags(void) {
	uint32_t cpuflags = LOCAL_PREFIX(cpuid)();

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
#else
static void
print_cpuflags(void) {
	printf("generic cpu\n");
}
#endif /* defined(ARCH_X86) */

static void
try_example(void) {
	int32_t arr[127];
	int32_t i;
	for (i = 0; i < 127; i++)
		arr[i] = (i + 93);
	printf("sum of (93..219) = %d (check vs http://www.wolframalpha.com/input/?i=sum+of+93+to+219)\n", example(arr, 127));
}

int main(void) {
	if (example_init() != 0) {
		printf("example failed to initialize\n");
		return 1;
	}
	print_cpuflags();
	try_example();
	return 0;
}
