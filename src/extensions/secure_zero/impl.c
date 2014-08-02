#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cpuid.h"
#include "secure_zero.h"

typedef struct secure_zero_extension_t {
	unsigned long cpu_flags;
	const char *desc;
	void (*secure_zero)(unsigned char *p, size_t len);
} secure_zero_extension_t;

#define DECLARE_SECURE_ZERO_EXTENSION(ext) void secure_zero_##ext(unsigned char *p, size_t len);

#if defined(ARCH_X86)
#endif

#include "secure_zero/secure_zero_generic.h"

#define ADD_SECURE_ZERO_EXTENSION(flags, desc, ext) {flags, desc, secure_zero_##ext}

static const secure_zero_extension_t secure_zero_list[] = {
	ADD_SECURE_ZERO_EXTENSION(CPUID_GENERIC, "generic", generic)
};

static void secure_zero_bootup(unsigned char *p, size_t len);

static const secure_zero_extension_t secure_zero_bootup_ext = {
	CPUID_GENERIC,
	NULL,
	secure_zero_bootup
};

static const secure_zero_extension_t *secure_zero_opt = &secure_zero_bootup_ext;

LIB_PUBLIC void
secure_zero(unsigned char *p, size_t len) {
	secure_zero_opt->secure_zero(p, len);
}

static int
secure_zero_test_impl_len(const secure_zero_extension_t *ext, size_t offset, size_t len) {
	unsigned char arr[136];
	size_t i;
	int ret = 0;

	for (i = 0; i < sizeof(arr); i++)
		arr[i] = (unsigned char)~i;

	ext->secure_zero(arr + offset, len);

	/* prefix */
	for (i = 0; i < offset; i++)
		if (arr[i] != (unsigned char)~i)
			ret |= 1;

	/* body */
	for (i = offset; i < (len + offset); i++)
		if (arr[i] != 0)
			ret |= 1;

	/* suffix */
	for (i = len + offset; i < sizeof(arr); i++)
		if (arr[i] != (unsigned char)~i)
			ret |= 1;

	return ret;
}

static int
secure_zero_test_impl(const void *impl) {
	const secure_zero_extension_t *ext = (const secure_zero_extension_t *)impl;
	size_t i;
	int ret = 0;

	for (i = 0; i < 128; i++)
		ret |= secure_zero_test_impl_len(ext, 4, i);
	for (i = 1; i < 16; i++)
		ret |= secure_zero_test_impl_len(ext, i, 57 + i);
	return ret;
}

LIB_PUBLIC int
secure_zero_init(void) {
	const void *opt = LOCAL_PREFIX(cpu_select)(secure_zero_list, sizeof(secure_zero_extension_t), secure_zero_test_impl);
	if (opt) {
		secure_zero_opt = (const secure_zero_extension_t *)opt;
		return 0;
	} else {
		return 1;
	}
}

static void
secure_zero_bootup(unsigned char *p, size_t len) {
	if (secure_zero_init() == 0) {
		secure_zero_opt->secure_zero(p, len);
	} else {
		fprintf(stderr, "secure_zero failed to initialize\n");
		exit(1);
	}
}




#if defined(UTILITIES)

#include "util/bench.h"
#include "util/fuzz.h"

static const fuzz_variable_t fuzz_inputs[] = {
	{"input", FUZZ_RANDOM_LENGTH_ARRAY0, 16384},
	{0, FUZZ_DONE, 0}
};

static const fuzz_variable_t fuzz_outputs[] = {
	{"output", FUZZ_RANDOM_LENGTH_ARRAY0, 0},
	{0, FUZZ_DONE, 0}
};


/* process the input with the given implementation and write it to the output */
static void
secure_zero_fuzz_impl(const void *impl, const unsigned char *in, const size_t *random_sizes, unsigned char *out) {
	const secure_zero_extension_t *ext = (const secure_zero_extension_t *)impl;
	size_t bytes;

	/* get count for random array 0 */
	bytes = random_sizes[0];

	/* process the data */
	memcpy(out, in, bytes);
	ext->secure_zero(out, bytes);
	out += bytes;
}

/* run the fuzzer on secure_zero */
void
secure_zero_fuzz(void) {
	fuzz_init();
	fuzz(secure_zero_list, sizeof(secure_zero_extension_t), fuzz_inputs, fuzz_outputs, secure_zero_fuzz_impl);
}


static unsigned char *bench_arr = NULL;
static size_t bench_len = 0;

static void
secure_zero_bench_impl(const void *impl) {
	const secure_zero_extension_t *ext = (const secure_zero_extension_t *)impl;
	ext->secure_zero(bench_arr, bench_len);
}

void
secure_zero_bench(void) {
	static const size_t lengths[] = {7, 16, 32, 40, 50, 64, 128, 256, 4096, 0};
	static const char *titles[] = {"aligned", "unaligned"};
	size_t i, j;
	bench_arr = (unsigned char *)bench_get_buffer();
	for (i = 0; i < 2; i++) {
		printf("%s test\n", titles[i]);
		for (j = 0; lengths[j]; j++) {
			bench_len = lengths[j];
			bench(secure_zero_list, sizeof(secure_zero_extension_t), secure_zero_test_impl, secure_zero_bench_impl, bench_len, "byte");
		}
		bench_arr++;
	}
}



#endif /* defined(UTILITIES) */




