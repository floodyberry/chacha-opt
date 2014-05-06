#include <stddef.h>
#include "cpuid.h"
#include "example/include.h"

/* int example(const int *arr, size_t count) returns the sum of count ints in arr */

typedef struct example_impl_t {
	unsigned long cpu_flags;
	int (*example)(const int *arr, size_t count);
} example_impl_t;

/* declare the prototypes of the provided functions */
#define EXAMPLE_DECLARE(ext) \
	extern int example_##ext(const int *arr, size_t count);

#if defined(ARCH_X86)
	/* 32 bit only implementations */
	#if defined(CPU_32BITS)
		EXAMPLE_DECLARE(x86)
		#define EXAMPLE_X86 {CPUID_X86, example_x86}
	#endif

	/* 64 bit only implementations */
	#if defined(CPU_64BITS)
		#if defined(HAVE_AVX)
			EXAMPLE_DECLARE(avx)
			#define EXAMPLE_AVX {CPUID_AVX, example_avx}
		#endif
	#endif

	/* both 32 & 64 bit implementations */
	#if defined(HAVE_SSE2)
		EXAMPLE_DECLARE(sse2)
		#define EXAMPLE_SSE2  {CPUID_SSE2, example_sse2}
	#endif
#endif

/* the "always runs" version */
#define EXAMPLE_GENERIC {CPUID_GENERIC, example_generic}
#include "example_generic.h"

/* list implemenations from most optimized to least, with generic as the last entry */
static const example_impl_t example_list[] = {
	#if defined(EXAMPLE_AVX)
		EXAMPLE_AVX,
	#endif
	#if defined(EXAMPLE_SSE2)
		EXAMPLE_SSE2,
	#endif
	#if defined(EXAMPLE_X86)
		EXAMPLE_X86,
	#endif
	EXAMPLE_GENERIC
};

static example_impl_t example_opt = {0,0};

/* test an implementation */
static int
example_test(const void *impl) {
	const example_impl_t *example_impl = (const example_impl_t *)impl;
	int arr[50], i;
	for (i = 0; i < 50; i++)
		arr[i] = i;
	return (example_impl->example(arr, 50) == 1225) ? 0 : 1;
}

/* choose the best implemenation for the current cpu */
int
example_init(void) {
	const void *opt = cpu_select(example_list, sizeof(example_impl_t), example_test);
	if (opt) {
		example_opt = *(const example_impl_t *)opt;
		return 0;
	} else {
		return 1;
	}
}

/* call the optimized implementation */
int
example(const int *arr, size_t count) {
	return example_opt.example(arr, count);
}

/* tests all available implementations */
int
example_test_all(void) {
	return cpu_test_all(example_list, sizeof(example_impl_t), example_test);
}
