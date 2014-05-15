#include <stddef.h>
#include "cpuid.h"
#include "example/include.h"

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
example_test(const void *impl) {
	const example_impl_t *example_impl = (const example_impl_t *)impl;
	int32_t arr[50], i;
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
int32_t
example(const int32_t *arr, size_t count) {
	return example_opt.example(arr, count);
}

#if defined(FUZZ)

#include <stdio.h>
#include <string.h>
#include "fuzz.h"

/* setup a fuzz pass, generate random data for the input, and tell the fuzzer how much output to expect */
static void
example_fuzz_setup(uint8_t *in, size_t *in_bytes, size_t *out_bytes) {
	uint8_t *in_start = in;
	size_t arr_len;
	fuzz_get_bytes(&arr_len, sizeof(arr_len));

	/* use an array size of 0->16384 bytes / sizeof(int32_t) = number of ints to count up */
	arr_len = (arr_len % 16384) / sizeof(int32_t);
	memcpy(in, &arr_len, sizeof(arr_len));
	in += sizeof(arr_len);

	/* generate the input ints! */
	fuzz_get_bytes(in, arr_len * sizeof(int32_t));
	in += arr_len * sizeof(int32_t);

	/* amount of input that will be used */
	*in_bytes = in - in_start;

	/* amount of output each implementation will produce */
	*out_bytes = sizeof(int32_t);
}


/* process the input with the given implementation and write it to the output */
static size_t
example_fuzz(const void *impl, const uint8_t *in, uint8_t *out) {
	const example_impl_t *example_impl = (const example_impl_t *)impl;
	uint8_t *out_start = out;
	size_t int_count;
	int32_t sum;

	/* read count */
	memcpy(&int_count, in, sizeof(int_count));
	in += sizeof(int_count);

	/* sum the array */
	sum = example_impl->example((const int32_t *)in, int_count);

	/* store the result */
	memcpy(out, &sum, sizeof(sum));
	out += sizeof(sum);

	/* return bytes written */
	return (out - out_start);
}

/* print len bytes from bytes in hex format, xor'd against base if base != bytes */
static void
example_fuzz_print_bytes(const char *desc, const uint8_t *base, const uint8_t *bytes, size_t len) {
	size_t i;
	printf("%s: ", desc);
	for (i = 0; i < len; i++) {
		if (i && ((i % 16) == 0))
			printf("\n");
		if (base != bytes) {
			uint8_t diff = base[i] ^ bytes[i];
			if (diff)
				printf("0x%02x,", diff);
			else
				printf("____,");
		} else {
			printf("0x%02x,", bytes[i]);
		}
	}
	printf("\n\n");
}

/* print the output for the given implementation, and xor it against generic_out if needed */
static void
example_fuzz_print(const void *impl, const uint8_t *in, const uint8_t *out, const uint8_t *generic_out) {
	const example_impl_t *example_impl = (const example_impl_t *)impl;
	if (out == generic_out) {
		size_t int_count;
		/* this is the generic data, print the input first */
		printf("INPUT\n\n");

		/* input length */
		memcpy(&int_count, in, sizeof(int_count));
		in += sizeof(int_count);
		printf("length: %u\n", (uint32_t)int_count);

		/* dump data */
		example_fuzz_print_bytes("data", in, in, int_count * sizeof(int32_t));

		/* switch to output! */
		printf("OUTPUT\n\n");
	}
	printf("IMPLEMENTATION:%s\n", example_impl->desc);
	example_fuzz_print_bytes("sum", generic_out, out, sizeof(int32_t));
	out += sizeof(int32_t);
	generic_out += sizeof(int32_t);
}

/* run the fuzzer on example */
void
example_fuzzer(void) {
	fuzz_init();
	fuzz(example_list, sizeof(example_impl_t), example_fuzz_setup, example_fuzz, example_fuzz_print);
}

#endif /* FUZZ */
