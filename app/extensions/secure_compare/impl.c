#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cpuid.h"
#include "secure_compare.h"

/*
	secure_compareX compares two X byte arrays in constant time and 
	returns 0 if they are equal, -1 if they are not
*/


typedef struct secure_compare_extension_t {
	unsigned long cpu_flags;
	const char *desc;
	int (*secure_compare8)(const unsigned char *x, const unsigned char *y);
	int (*secure_compare16)(const unsigned char *x, const unsigned char *y);
	int (*secure_compare32)(const unsigned char *x, const unsigned char *y);
} secure_compare_extension_t;

#define DECLARE_SECURE_COMPARE_EXTENSION(ext) \
	int secure_compare8_##ext(const unsigned char *x, const unsigned char *y); \
	int secure_compare16_##ext(const unsigned char *x, const unsigned char *y); \
	int secure_compare32_##ext(const unsigned char *x, const unsigned char *y);

#if defined(ARCH_X86)
	#if defined(CPU_32BITS)
		#if defined(HAVE_SSE2)
			#define SECURE_COMPARE_SSE2
			DECLARE_SECURE_COMPARE_EXTENSION(sse2)
		#endif
	#endif

	#define SECURE_COMPARE_X86
	DECLARE_SECURE_COMPARE_EXTENSION(x86)
#endif

#if defined(ARCH_ARM)
	#if defined(CPU_32BITS)
		#if defined(HAVE_ARMv7)
			#define SECURE_COMPARE_ARMv7
			DECLARE_SECURE_COMPARE_EXTENSION(armv7)
		#endif
	#endif
#endif

#include "secure_compare/secure_compare_generic.h"

#define ADD_SECURE_COMPARE_EXTENSION(flags, desc, ext) {flags, desc, secure_compare8_##ext, secure_compare16_##ext ,secure_compare32_##ext}

static const secure_compare_extension_t secure_compare_list[] = {
/* x86 */
#if defined(SECURE_COMPARE_SSE2)
	ADD_SECURE_COMPARE_EXTENSION(CPUID_SSE2, "sse2", sse2),
#endif
#if defined(SECURE_COMPARE_X86)
	ADD_SECURE_COMPARE_EXTENSION(CPUID_X86, "x86", x86),
#endif

/* arm */
#if defined(SECURE_COMPARE_ARMv7)
	ADD_SECURE_COMPARE_EXTENSION(CPUID_ARMv7, "armv7", armv7),
#endif

	ADD_SECURE_COMPARE_EXTENSION(CPUID_GENERIC, "generic", generic)
};

static int secure_compare8_bootup(const unsigned char *x, const unsigned char *y);
static int secure_compare16_bootup(const unsigned char *x, const unsigned char *y);
static int secure_compare32_bootup(const unsigned char *x, const unsigned char *y);

static const secure_compare_extension_t secure_compare_bootup_ext = {
	CPUID_GENERIC,
	NULL,
	secure_compare8_bootup,
	secure_compare16_bootup,
	secure_compare32_bootup
};

static const secure_compare_extension_t *secure_compare_opt = &secure_compare_bootup_ext;

LIB_PUBLIC int
secure_compare8(const unsigned char *x, const unsigned char *y) {
	return secure_compare_opt->secure_compare8(x, y);
}

LIB_PUBLIC int
secure_compare16(const unsigned char *x, const unsigned char *y) {
	return secure_compare_opt->secure_compare16(x, y);
}

LIB_PUBLIC int
secure_compare32(const unsigned char *x, const unsigned char *y) {
	return secure_compare_opt->secure_compare32(x, y);
}

static int
secure_compare_test_impl(const void *impl) {
	const secure_compare_extension_t *ext = (const secure_compare_extension_t *)impl;
	unsigned char x[32], y[32];
	size_t i, j;
	int ret = 0;

	/* simple equality test */
	for (i = 0; i < 32; i++) {
		unsigned int h = ((unsigned int)i ^ 0xcafe) * 0xbeef;
		h ^= h >> 8;
		x[i] = (unsigned char)h;
		y[i] = (unsigned char)h;
	}

	ret |= (ext->secure_compare8(x, y) != 0);
	ret |= (ext->secure_compare16(x, y) != 0);
	ret |= (ext->secure_compare32(x, y) != 0);


	/* flip each bit in x */
	for (i = 0; i < 32; i++) {
		for (j = 0; j < 7; j++) {
			x[i] ^= (1 << j);
			if (i < 8)
				ret |= (ext->secure_compare8(x, y) == 0);
			if (i < 16)
				ret |= (ext->secure_compare16(x, y) == 0);
			ret |= (ext->secure_compare32(x, y) == 0);
			x[i] ^= (1 << j);
		}
	}

	return ret;
}

LIB_PUBLIC int
secure_compare_init(void) {
	const void *opt = LOCAL_PREFIX(cpu_select)(secure_compare_list, sizeof(secure_compare_extension_t), secure_compare_test_impl);
	if (opt) {
		secure_compare_opt = (const secure_compare_extension_t *)opt;
		return 0;
	} else {
		return 1;
	}
}

static int
secure_compare8_bootup(const unsigned char *x, const unsigned char *y) {
	int ret = 1;
	if (secure_compare_init() == 0) {
		ret = secure_compare_opt->secure_compare8(x, y);
	} else {
		fprintf(stderr, "secure_zero failed to initialize\n");
		exit(1);
	}
	return ret;
}

static int
secure_compare16_bootup(const unsigned char *x, const unsigned char *y) {
	int ret = 1;
	if (secure_compare_init() == 0) {
		ret = secure_compare_opt->secure_compare16(x, y);
	} else {
		fprintf(stderr, "secure_zero failed to initialize\n");
		exit(1);
	}
	return ret;
}

static int
secure_compare32_bootup(const unsigned char *x, const unsigned char *y) {
	int ret = 1;
	if (secure_compare_init() == 0) {
		ret = secure_compare_opt->secure_compare32(x, y);
	} else {
		fprintf(stderr, "secure_zero failed to initialize\n");
		exit(1);
	}
	return ret;
}


#if defined(UTILITIES)

#include "bench.h"
#include "fuzz.h"

static const fuzz_variable_t fuzz_inputs[] = {
	{"input32x", FUZZ_ARRAY, 32},
	{"input32y", FUZZ_ARRAY, 32},
	{0, FUZZ_DONE, 0}
};

static const fuzz_variable_t fuzz_outputs[] = {
	{"output8same", FUZZ_ARRAY, 1},
	{"output8dif", FUZZ_ARRAY, 1},
	{"output16same", FUZZ_ARRAY, 1},
	{"output16dif", FUZZ_ARRAY, 1},
	{"output32same", FUZZ_ARRAY, 1},
	{"output32dif", FUZZ_ARRAY, 1},
	{0, FUZZ_DONE, 0}
};


/* process the input with the given implementation and write it to the output */
static void
secure_compare_fuzz_impl(const void *impl, const unsigned char *in, const size_t *random_sizes, unsigned char *out) {
	const secure_compare_extension_t *ext = (const secure_compare_extension_t *)impl;
	const unsigned char *x = in, *y = in + 32;
	(void)random_sizes;
	out[0] = (unsigned char)ext->secure_compare8(x, x);
	out[1] = (unsigned char)ext->secure_compare8(x, y);
	out[2] = (unsigned char)ext->secure_compare16(x, x);
	out[3] = (unsigned char)ext->secure_compare16(x, y);
	out[4] = (unsigned char)ext->secure_compare32(x, x);
	out[5] = (unsigned char)ext->secure_compare32(x, y);
}

/* run the fuzzer on secure_compare */
void
secure_compare_fuzz(void) {
	fuzz_init();
	fuzz(secure_compare_list, sizeof(secure_compare_extension_t), fuzz_inputs, fuzz_outputs, secure_compare_fuzz_impl);
}


static unsigned char *bench_arr = NULL;

static void
secure_compare32_bench_impl(const void *impl) {
	const secure_compare_extension_t *ext = (const secure_compare_extension_t *)impl;
	ext->secure_compare32(bench_arr, bench_arr + 384);
}

static void
secure_compare16_bench_impl(const void *impl) {
	const secure_compare_extension_t *ext = (const secure_compare_extension_t *)impl;
	ext->secure_compare16(bench_arr, bench_arr + 384);
}

static void
secure_compare8_bench_impl(const void *impl) {
	const secure_compare_extension_t *ext = (const secure_compare_extension_t *)impl;
	ext->secure_compare8(bench_arr, bench_arr + 384);
}

void
secure_compare_bench(void) {
	bench_arr = bench_get_buffer();
	bench(secure_compare_list, sizeof(secure_compare_extension_t), secure_compare_test_impl, secure_compare32_bench_impl, 32, "byte");
	bench(secure_compare_list, sizeof(secure_compare_extension_t), secure_compare_test_impl, secure_compare16_bench_impl, 16, "byte");
	bench(secure_compare_list, sizeof(secure_compare_extension_t), secure_compare_test_impl, secure_compare8_bench_impl, 8, "byte");
}



#endif /* defined(UTILITIES) */




