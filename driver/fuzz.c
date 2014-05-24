#if defined(UTILITIES)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "cpuid.h"
#include "fuzz.h"

/*
	Chacha/8 rng with no addition of state words post-mixing, no security at all, but good 
	portable random numbers for fuzzing
*/

/* store a 32 bit unsigned integer as four 8 bit unsigned integers in little endian */
static void
store8(uint8_t *p, uint32_t v) {
	p[0] = (v      ) & 0xff;
	p[1] = (v >>  8) & 0xff;
	p[2] = (v >> 16) & 0xff;
	p[3] = (v >> 24) & 0xff;
}

/* 32 bit left rotate */
static uint32_t
rotate32(uint32_t x, int k) {
	return ((x << k) | (x >> (32 - k)));
}

typedef struct chacha_state_t {
	uint32_t s[12];
} chacha_state_t;

/* 1 block = 64 bytes */
static void
chacha_blocks(chacha_state_t *state, uint8_t *out, size_t blocks) {
	uint32_t x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15;
	uint32_t             j4,j5,j6,j7,j8,j9,j10,j11,j12,j13,j14,j15;
	uint32_t t;
	size_t i;

	j4 = state->s[0];
	j5 = state->s[1];
	j6 = state->s[2];
	j7 = state->s[3];
	j8 = state->s[4];
	j9 = state->s[5];
	j10 = state->s[6];
	j11 = state->s[7];
	j12 = state->s[8];
	j13 = state->s[9];
	j14 = state->s[10];
	j15 = state->s[11];

	for ( ; blocks; blocks -= 1, out += 64) {
		/* "expand 32-byte k", as 4 little endian 32-bit unsigned integers */
		x0 = 0x61707865;
		x1 = 0x3320646e;
		x2 = 0x79622d32;
		x3 = 0x6b206574;
		x4 = j4;
		x5 = j5;
		x6 = j6;
		x7 = j7;
		x8 = j8;
		x9 = j9;
		x10 = j10;
		x11 = j11;
		x12 = j12;
		x13 = j13;
		x14 = j14;
		x15 = j15;

		#define quarter(a,b,c,d) \
			a = (a + b); t = d^a; d = rotate32(t,16); \
			c = (c + d); t = b^c; b = rotate32(t,12); \
			a = (a + b); t = d^a; d = rotate32(t, 8); \
			c = (c + d); t = b^c; b = rotate32(t, 7);

		for (i = 0; i < 8; i += 2) {
			quarter( x0, x4, x8,x12)
			quarter( x1, x5, x9,x13)
			quarter( x2, x6,x10,x14)
			quarter( x3, x7,x11,x15)
			quarter( x0, x5,x10,x15)
			quarter( x1, x6,x11,x12)
			quarter( x2, x7, x8,x13)
			quarter( x3, x4, x9,x14)
		}

		store8(out +  0,  x0);
		store8(out +  4,  x1);
		store8(out +  8,  x2);
		store8(out + 12,  x3);
		store8(out + 16,  x4);
		store8(out + 20,  x5);
		store8(out + 24,  x6);
		store8(out + 28,  x7);
		store8(out + 32,  x8);
		store8(out + 36,  x9);
		store8(out + 40, x10);
		store8(out + 44, x11);
		store8(out + 48, x12);
		store8(out + 52, x13);
		store8(out + 56, x14);
		store8(out + 60, x15);

		/* use counter+iv as a 128 bit counter */
		j12 = (j12 + 1);
		if (!j12) {
			j13 = (j13 + 1);
			if (!j13) {
				j14 = (j14 + 1);
				if (!j14)
					j15 = (j15 + 1);
			}
		}
	}

	state->s[8] = j12;
	state->s[9] = j13;
	state->s[10] = j14;
	state->s[11] = j15;
}

typedef struct fuzz_state_t {
	chacha_state_t rng;
	uint8_t buffer[64];
	size_t remaining;
} fuzz_state_t;

static fuzz_state_t fuzz_state;

/* reload the fuzz random number buffer */
static void
fuzz_reload(fuzz_state_t *st) {
	chacha_blocks(&st->rng, st->buffer, sizeof(st->buffer) / 64);
	st->remaining = sizeof(st->buffer);
}

/* initialize the state to all zeros */
void
fuzz_init_deterministic(void) {
	memset(&fuzz_state.rng, 0, sizeof(fuzz_state.rng));
	fuzz_reload(&fuzz_state);
}

/* initialize the state randomly */
void
fuzz_init(void) {
#if (defined(_WIN32) || defined(_WIN64))
	HCRYPTPROV handle;
	if (!CryptAcquireContext(&handle, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
		fprintf(stderr, "CryptAcquireContext failed");
		exit(1);
	}
	CryptGenRandom(handle, sizeof(fuzz_state.rng), (BYTE*)&fuzz_state.rng);
	CryptReleaseContext(handle, 0);
#else
	FILE *f = fopen("/dev/urandom", "r");
	if (!f) {
		fprintf(stderr, "failed to open /dev/urandom");
		exit(1);
	}
	if (fread(&fuzz_state.rng, sizeof(fuzz_state.rng), 1, f) != 1) {
		fprintf(stderr, "read on /dev/urandom failed");
		exit(1);
	}
	fclose(f);
#endif
	fuzz_reload(&fuzz_state);
}

/* get len random bytes */
void
fuzz_get_bytes(void *out, size_t len) {
	uint8_t *outb = (uint8_t *)out;

	while (len) {
		/* drain the stored buffer first */
		if (fuzz_state.remaining) {
			size_t bytes = (len > fuzz_state.remaining) ? fuzz_state.remaining : len;
			memcpy(outb, fuzz_state.buffer + (sizeof(fuzz_state.buffer) - fuzz_state.remaining), bytes);

			fuzz_state.remaining -= bytes;
			outb += bytes;
			len -= bytes;
		}

		/* fill up with full blocks */
		if (len >= 64) {
			size_t bytes = (len & ~63), blocks = len / 64;
			chacha_blocks(&fuzz_state.rng, outb, blocks);
			outb += bytes;
			len -= bytes;
		}

		/* refill the stored buffer if needed */
		if (!fuzz_state.remaining)
			fuzz_reload(&fuzz_state);
	}
}

/* print len bytes from bytes in hex format, xor'd against base if bytes != base */
void
fuzz_print_bytes(const char *desc, const uint8_t *bytes, const uint8_t *base, size_t len) {
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

/* run the fuzzer */
void
fuzz(const void *impls, size_t impl_size, impl_fuzz_setup setup_fn, impl_fuzz fuzz_fn, impl_fuzz_print print_fn) {
	/* allocate data */
	uint8_t *fuzz_input = NULL, *fuzz_output = NULL;
	const cpu_specific_impl_t **impl_list_alloc = (const cpu_specific_impl_t **)malloc(sizeof(const cpu_specific_impl_t *) * 32), **impl_list;
	size_t impl_count = 0;

	/* cpu detection */
	uint32_t cpu_flags = cpuid();
	const char *p = (const char *)impls;

	size_t expected_bytes_in, expected_bytes_out;
	uint8_t *outp;
	size_t i;

	/* counter display */
	clock_t start, clocks;
	size_t counter, counter_dot, counter_line;
	int display_counter = 0;

	/* aggregate number of implementations, storing them in reverse order (generic first, most optimized last) */
	impl_list = &impl_list_alloc[31];
	for (;;) {
		const cpu_specific_impl_t *impl = (const cpu_specific_impl_t *)p;
		if (impl->cpu_flags == (impl->cpu_flags & cpu_flags))
			*(impl_list--) = (const cpu_specific_impl_t *)impl;
		if (impl->cpu_flags == CPUID_GENERIC)
			break;
		p += impl_size;
	}

	/* need at least 2 added to do anything interesting */
	impl_count = (&impl_list_alloc[31] - impl_list);
	if (impl_count <= 1) {
		printf("not enough implementations to fuzz..\n");
		goto done;
	}
	/* point it at the last impl added */
	impl_list += 1; 

	/* 16k for raw data, 1k for key material and derived data */
	fuzz_input = (uint8_t *)malloc(16384 + 1024); 
	fuzz_output = (uint8_t *)malloc((16384 + 1024) * impl_count);

	/* show list of implementations being fuzzed */
	printf("fuzzing %s", impl_list[0]->desc);
	for (i = 1; i < impl_count; i++) {
		printf(", %s", impl_list[i]->desc);
	}
	printf("\n\n");

	/* fuzz loop */
	counter = 0;
	start = clock();
	for (;;) {
		uint8_t *generic_out = fuzz_output;

		/* set up the data for this run */
		setup_fn(fuzz_input, &expected_bytes_in, &expected_bytes_out);

		/* gather results */
		outp = fuzz_output;
		for (i = 0; i < impl_count; i++) {
			size_t written = fuzz_fn(impl_list[i], fuzz_input, outp);
			if (written != expected_bytes_out) {
				printf("%s: didn't produce the expected number of bytes!\n", impl_list[i]->desc);
				goto done;
			}
			outp += expected_bytes_out;
		}

		/* compare results */
		outp = fuzz_output + expected_bytes_out;
		for (i = 1; i < impl_count; i++) {
			if (memcmp(generic_out, outp, expected_bytes_out) != 0)
				goto failure;
		}

		counter++;

		/* are we still calibrating? */
		if (!display_counter) {
			clocks = clock();
			if (clocks == (clock_t)-1) {
				/* clock is broken, use values which might suck.. */
				counter_line = 8192;
				counter_dot = (counter_line / 32);
				counter = 0;
				display_counter = 1;
			} else if ((clocks - start) >= CLOCKS_PER_SEC) {
				printf("doing approximately %u passes a second..\n", (unsigned int)(counter));

				/* 32 dots per line, 1 line per ~5 seconds */
				counter_line = 1;
				counter *= 5;
				while (counter_line < counter)
					counter_line *= 2;
				if (counter_line < 32)
					counter_line = 32;
				counter_dot = (counter_line / 32);
				if (counter_dot < 1)
					counter_dot = 1;

				counter = 0;
				display_counter = 1;
			}
		} else {
			if ((counter & (counter_dot - 1)) == 0)
				printf(".");
			if ((counter & (counter_line - 1)) == 0)
				printf("[%08x]\n", (unsigned int)(counter));
		}
	}

failure:
	printf("fuzz mismatch! dumping input and output data\n\n");

	outp = fuzz_output;
	print_fn(impl_list[0], fuzz_input, outp, fuzz_output);
	outp += expected_bytes_out;

	for (i = 1; i < impl_count; i++) {
		print_fn(impl_list[i], fuzz_input, outp, fuzz_output);
		outp += expected_bytes_out;
	}

done:
	if (fuzz_input)
		free(fuzz_input);
	if (fuzz_output)
		free(fuzz_output);
	free(impl_list_alloc);
}

#endif /* defined(UTILITIES) */
