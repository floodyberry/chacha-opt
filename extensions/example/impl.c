#include <stddef.h>
#include "cpuid.h"
#include "example.h"

/* int example(const int *arr, size_t count) returns the sum of count ints in arr */

typedef struct example_impl_t {
	uint32_t cpu_flags;
	const char *desc;
	int32_t (*example)(const int32_t *arr, size_t count);
} example_impl_t;

/* declare the prototypes of the provided functions */
#define EXAMPLE_DECLARE(ext) \
	extern int32_t example_##ext(const int32_t *arr, size_t count);

#if defined(ARCH_X86)
	/* 32 bit only implementations */
	#if defined(CPU_32BITS)
		EXAMPLE_DECLARE(x86)
		#define EXAMPLE_X86 {CPUID_X86, "x86", example_x86}
	#endif

	/* 64 bit only implementations */
	#if defined(CPU_64BITS)
		#if defined(HAVE_AVX)
			EXAMPLE_DECLARE(avx)
			#define EXAMPLE_AVX {CPUID_AVX, "avx", example_avx}
		#endif
	#endif

	/* both 32 & 64 bit implementations */
	#if defined(HAVE_SSE2)
		EXAMPLE_DECLARE(sse2)
		#define EXAMPLE_SSE2  {CPUID_SSE2, "sse2", example_sse2}
	#endif
#endif

/* the "always runs" version */
#define EXAMPLE_GENERIC {CPUID_GENERIC, "generic", example_generic}
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

static example_impl_t example_opt = {0,0,0};

/* test an implementation */
static int
example_test_impl(const void *impl) {
	const example_impl_t *example_impl = (const example_impl_t *)impl;
	int32_t arr[50], i, sum;
	int ret = 0;

	for (i = 0, sum = 0; i < 50; i++) {
		arr[i] = i;
		sum += i;
	}
	for (i = 0; i <= 50; i++) {
		ret |= (example_impl->example(arr, 50 - i) == sum) ? 0 : 1;
		sum -= (50 - i - 1);
	}
	return ret;
}

/* choose the best implemenation for the current cpu */
int
example_init(void) {
	const void *opt = cpu_select(example_list, sizeof(example_impl_t), example_test_impl);
	if (opt) {
		example_opt = *(const example_impl_t *)opt;
		return 0;
	} else {
		return 1;
	}
}

/* call the optimized implementation */
int32_t
example(const int32_t *arr, size_t count) {
	return example_opt.example(arr, count);
}

#if defined(UTILITIES)

#include <stdio.h>
#include <string.h>
#include "util/fuzz.h"
#include "util/bench.h"

static fuzz_variable_t fuzz_inputs[] = {
	{"input", FUZZ_RANDOM_LENGTH_ARRAY0, 16384},
	{0, FUZZ_DONE, 0}
};

static fuzz_variable_t fuzz_outputs[] = {
	{"sum", FUZZ_INT32, 1},
	{0, FUZZ_DONE, 0}
};


/* process the input with the given implementation and write it to the output */
static void
example_fuzz_impl(const void *impl, const uint8_t *in, const size_t *random_sizes, uint8_t *out) {
	const example_impl_t *example_impl = (const example_impl_t *)impl;
	size_t int_count;
	int32_t sum;

	/* get count of random array 0 */
	int_count = random_sizes[0] / sizeof(int32_t);

	/* sum the array */
	sum = example_impl->example((const int32_t *)in, int_count);

	/* store the result */
	memcpy(out, &sum, sizeof(sum));
	out += sizeof(sum);
}

/* run the fuzzer on example */
void
example_fuzz(void) {
	fuzz_init();
	fuzz(example_list, sizeof(example_impl_t), fuzz_inputs, fuzz_outputs, example_fuzz_impl);
}



static int32_t *bench_arr = NULL;
static size_t bench_len = 0;
static const size_t bench_trials = 10000000;

static void
example_bench_impl(const void *impl) {
	const example_impl_t *example_impl = (const example_impl_t *)impl;
	example_impl->example(bench_arr, bench_len);
}

void
example_bench(void) {
	static const size_t lengths[] = {16, 256, 4096, 0};
	size_t i;
	bench_arr = (int32_t *)bench_get_buffer();
	memset(bench_arr, 0xf5, 32768);
	for (i = 0; lengths[i]; i++) {
		bench_len = lengths[i];
		bench(example_list, sizeof(example_impl_t), example_test_impl, example_bench_impl, bench_len, "byte", bench_trials / ((bench_len / 100) + 1));
	}
}

#endif /* defined(UTILITIES) */
