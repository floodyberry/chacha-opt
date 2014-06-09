#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cpuid.h"
#include "secure_zero.h"

typedef struct secure_zero_extension_t {
	uint32_t cpu_flags;
	const char *desc;
	void (*secure_zero)(uint8_t *p, size_t len);
} secure_zero_extension_t;

#define DECLARE_SECURE_ZERO_EXTENSION(ext) void secure_zero_##ext(uint8_t *p, size_t len);

#if defined(ARCH_X86)
	#if defined(CPU_32BITS)
		#if defined(HAVE_SSE)
			#define SECURE_ZERO_SSE
			DECLARE_SECURE_ZERO_EXTENSION(sse)
		#endif
	#endif

	#define SECURE_ZERO_X86
	DECLARE_SECURE_ZERO_EXTENSION(x86)
#endif

#include "secure_zero/secure_zero_generic.inc"

#define ADD_SECURE_ZERO_EXTENSION(flags, desc, ext) {flags, desc, secure_zero_##ext}

static const secure_zero_extension_t secure_zero_list[] = {
#if defined(SECURE_ZERO_SSE)
	ADD_SECURE_ZERO_EXTENSION(CPUID_SSE, "sse", sse),
#endif
#if defined(SECURE_ZERO_X86)
	ADD_SECURE_ZERO_EXTENSION(CPUID_X86, "x86", x86),
#endif
	ADD_SECURE_ZERO_EXTENSION(CPUID_GENERIC, "generic", generic)
};

static void secure_zero_bootup(uint8_t *p, size_t len);

static const secure_zero_extension_t secure_zero_bootup_ext = {
	CPUID_GENERIC,
	NULL,
	secure_zero_bootup
};

static const secure_zero_extension_t *secure_zero_opt = &secure_zero_bootup_ext;

void
secure_zero(uint8_t *p, size_t len) {
	secure_zero_opt->secure_zero(p, len);
}

static int
secure_zero_test_impl_len(const secure_zero_extension_t *ext, size_t offset, size_t len) {
	uint8_t arr[136];
	size_t i;
	int ret = 0;

	for (i = 0; i < sizeof(arr); i++)
		arr[i] = (uint8_t)~i;

	ext->secure_zero(arr + offset, len);

	/* prefix */
	for (i = 0; i < offset; i++)
		if (arr[i] != (uint8_t)~i)
			ret |= 1;

	/* body */
	for (i = offset; i < (len + offset); i++)
		if (arr[i] != 0)
			ret |= 1;

	/* suffix */
	for (i = len + offset; i < sizeof(arr); i++)
		if (arr[i] != (uint8_t)~i)
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

int
secure_zero_init(void) {
	const void *opt = cpu_select(secure_zero_list, sizeof(secure_zero_extension_t), secure_zero_test_impl);
	if (opt) {
		secure_zero_opt = (const secure_zero_extension_t *)opt;
		return 0;
	} else {
		return 1;
	}
}

static void
secure_zero_bootup(uint8_t *p, size_t len) {
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

static void
secure_zero_fuzz_setup(uint8_t *in, size_t *in_bytes, size_t *out_bytes) {
	uint8_t *in_start = in;
	size_t bytes;
	fuzz_get_bytes(&bytes, sizeof(bytes));

	/* use an array size of 0->512 bytes */
	bytes = (bytes % 512);
	memcpy(in, &bytes, sizeof(bytes));
	in += sizeof(bytes);

	/* generate the input ints! */
	fuzz_get_bytes(in, bytes);
	in += bytes;

	/* amount of input that will be used */
	*in_bytes = in - in_start;

	/* amount of output each implementation will produce */
	*out_bytes = bytes;
}


/* process the input with the given implementation and write it to the output */
static size_t
secure_zero_fuzz_impl(const void *impl, const uint8_t *in, uint8_t *out) {
	const secure_zero_extension_t *ext = (const secure_zero_extension_t *)impl;
	uint8_t *out_start = out;
	size_t bytes;

	/* read bytes */
	memcpy(&bytes, in, sizeof(bytes));
	in += sizeof(bytes);

	/* process the data */
	memcpy(out, in, bytes);
	ext->secure_zero(out, bytes);
	out += bytes;

	/* return bytes written */
	return (out - out_start);
}


/* print the output for the given implementation, and xor it against generic_out if needed */
static void
secure_zero_fuzz_print(const void *impl, const uint8_t *in, const uint8_t *out, const uint8_t *generic_out) {
	const secure_zero_extension_t *ext = (const secure_zero_extension_t *)impl;
	size_t bytes;

	/* input length */
	memcpy(&bytes, in, sizeof(bytes));
	in += sizeof(bytes);

	if (out == generic_out) {
		/* this is the generic data, print the input first */
		printf("INPUT\n\n");

		printf("length: %u\n", (uint32_t)bytes);

		/* dump data */
		fuzz_print_bytes("data", in, in, bytes);

		/* switch to output! */
		printf("OUTPUT\n\n");
	}
	printf("IMPLEMENTATION:%s\n", ext->desc);
	fuzz_print_bytes("output", out, generic_out, bytes);
	out += bytes;
	generic_out += bytes;
}

/* run the fuzzer on secure_zero */
void
secure_zero_fuzz(void) {
	fuzz_init();
	fuzz(secure_zero_list, sizeof(secure_zero_extension_t), secure_zero_fuzz_setup, secure_zero_fuzz_impl, secure_zero_fuzz_print);
}


static uint8_t *bench_arr = NULL;
static size_t bench_len = 0;
static const size_t bench_trials = 10000000;

static void
secure_zero_bench_impl(const void *impl) {
	const secure_zero_extension_t *ext = (const secure_zero_extension_t *)impl;
	ext->secure_zero(bench_arr, bench_len);
}

void
secure_zero_bench(void) {
	static const size_t lengths[] = {16, 256, 4096, 0};
	size_t i;
	bench_arr = (uint8_t *)bench_get_buffer();
	for (i = 0; lengths[i]; i++) {
		bench_len = lengths[i];
		bench(secure_zero_list, sizeof(secure_zero_extension_t), secure_zero_test_impl, secure_zero_bench_impl, bench_len, "byte", bench_trials / ((bench_len / 100) + 1));
	}
}



#endif /* defined(UTILITIES) */




