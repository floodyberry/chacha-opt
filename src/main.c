#include <stdio.h>
#include <string.h>
#include "cpuid.h"
#include "example.h"

#if defined(ARCH_X86)
static void
print_cpuflags(void) {
	unsigned long cpuflags = LOCAL_PREFIX(cpuid)();

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
#elif defined(ARCH_ARM)
static void
print_cpuflags(void) {
	unsigned long cpuflags = LOCAL_PREFIX(cpuid)();

	printf("cpu: ARM\n");

	if (cpuflags & (CPUID_ARMv6 | CPUID_ARMv7 | CPUID_ARMv8)) {
		printf("cpu compatible-ish with:\n");
		if (cpuflags & CPUID_ARMv6)
			printf(" ARMv6");
		if (cpuflags & CPUID_ARMv7)
			printf(" ARMv7");
		if (cpuflags & CPUID_ARMv8)
			printf(" ARMv8");
		printf("\n");
	}

	printf("cpu extensions:\n");
	if (cpuflags & CPUID_NEON)
		printf(" neon");
	if (cpuflags & CPUID_ASIMD)
		printf(" asimd");
	if (cpuflags & CPUID_IWMMXT)
		printf(" iwmmxt");

	if (cpuflags & CPUID_TLS)
		printf(" tls");
	if (cpuflags & CPUID_CRC32)
		printf(" crc32");

	if (cpuflags & CPUID_IDIVT)
		printf(" idivt");
	if (cpuflags & CPUID_IDIVA)
		printf(" idiva");

	if (cpuflags & CPUID_VFP3)
		printf(" vfp3");
	if (cpuflags & CPUID_VFP4)
		printf(" vfp4");
	if (cpuflags & CPUID_VFP3D16)
		printf(" vfp3d16");

	printf("\n");

	if (cpuflags & (CPUID_AES | CPUID_PMULL | CPUID_SHA1 | CPUID_SHA2)) {
		printf("crypto extensions: ");

		if (cpuflags & CPUID_AES)
			printf(" aes");
		if (cpuflags & CPUID_PMULL)
			printf(" pmull");
		if (cpuflags & CPUID_SHA1)
			printf(" sha1");
		if (cpuflags & CPUID_SHA2)
			printf(" sha2");
		printf("\n");
	}
}
#else
static void
print_cpuflags(void) {
	printf("generic cpu\n");
}
#endif /* defined(ARCH_X86) */

static void
try_example(void) {
	unsigned char arr[127];
	size_t i;
	for (i = 0; i < 127; i++)
		arr[i] = (unsigned char)(i + 93);
	printf("sum of (93..219) %% 256 = %d (check vs http://www.wolframalpha.com/input/?i=sum+of+93+to+219+mod+256)\n", example(arr, 127));
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
