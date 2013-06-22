/* needs stdio.h, string.h, chacha.h */

#define trials 32768

static uint8_t __attribute__((aligned(16))) buffer[2][16384];
static uint64_t allticks[trials+1];
static uint64_t minticks;

static void
chacha_bench() {
	static const chacha_key key = {{1}};
	static const chacha_iv iv = {{2}};
	static const chacha_key x_key = {{3}};
	static const chacha_iv24 x_iv = {{4}};
	static const size_t lengths[] = {1, 128, 576, 2048, 8192, 0};
	static const size_t rounds[] = {8, 12, 20, 0};
	size_t i, j, pass;

	if (!chacha_check_validity()) {
		printf("self check FAILED\n");
		return;
	} else {
		printf("self check passed\n\n");
	}

	memset(buffer, 0x5c, sizeof(buffer));

	for (i = 0; i < trials; i++)
		chacha(&key, &iv, buffer[0], buffer[1], 16384, 20);

	/* chacha */
	printf("chacha\n\n");
	for (i = 0; rounds[i] != 0; i++) {
		printf("%u rounds\n", (unsigned int)rounds[i]);
		for (j = 0; lengths[j] != 0; j++) {
			for (pass = 0; pass < trials; pass++) {
				allticks[pass] = get_ticks();
				chacha(&key, &iv, buffer[0], buffer[1], lengths[j], rounds[i]);
			}
			allticks[pass] = get_ticks();
			for (pass = 0; pass < trials; pass++)
				allticks[pass] = allticks[pass + 1] - allticks[pass];

			for (pass = 0, minticks = ~0ull; pass < trials; pass++) {
				if (allticks[pass] < minticks)
					minticks = allticks[pass];
			}

			printf("%u bytes, %.0f cycles, %.2f cycles/byte\n", (unsigned int)lengths[j], (double)minticks, (double)minticks / lengths[j]);
		}
		printf("\n");
	}

	printf("\nxchacha\n");
	for (i = 0; rounds[i] != 0; i++) {
		printf("\n%u rounds\n", (unsigned int)rounds[i]);
		for (j = 0; lengths[j] != 0; j++) {
			for (pass = 0; pass < trials; pass++) {
				allticks[pass] = get_ticks();
				xchacha(&x_key, &x_iv, buffer[0], buffer[1], lengths[j], rounds[i]);
			}
			allticks[pass] = get_ticks();
			for (pass = 0; pass < trials; pass++)
				allticks[pass] = allticks[pass + 1] - allticks[pass];

			for (pass = 0, minticks = ~0ull; pass < trials; pass++) {
				if (allticks[pass] < minticks)
					minticks = allticks[pass];
			}

			printf("%u bytes, %.0f cycles, %.2f cycles/byte\n", (unsigned int)lengths[j], (double)minticks, (double)minticks / lengths[j]);
		}
		printf("\n");
	}

	/* hchacha */
	printf("hchacha\n\n");
	for (i = 0; rounds[i] != 0; i++) {
		for (pass = 0; pass < trials; pass++) {
			allticks[pass] = get_ticks();
			hchacha(x_key.b, x_iv.b, buffer[0], rounds[i]);
		}
		allticks[pass] = get_ticks();
		for (pass = 0; pass < trials; pass++)
			allticks[pass] = allticks[pass + 1] - allticks[pass];

		for (pass = 0, minticks = ~0ull; pass < trials; pass++) {
			if (allticks[pass] < minticks)
				minticks = allticks[pass];
		}

		printf("%u rounds, %.0f cycles\n", (unsigned int)rounds[i], (double)minticks);
	}
}
