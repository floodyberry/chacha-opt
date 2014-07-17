#include <stdio.h>
#include "cpucycles.h"
#include "cpuid.h"
#include "util/bench.h"

/* a 32k, 64 byte aligned buffer to bench with */
unsigned char *bench_get_buffer(void) {
	static unsigned char buffer[0x8000 + 0x40 + 0x40];
	unsigned char *p = buffer;
	p += 0x3f;
	p -= (size_t)p & 0x3f;
	return p;
}

int
bench(const void *impls, size_t impl_size, impl_test test_fn, impl_bench bench_fn, size_t units_count, const char *units_desc, size_t trials) {
	unsigned long cpu_flags = LOCAL_PREFIX(cpuid)();
	const unsigned char *p = (const unsigned char *)impls;
	int first_item = 1;

	if (trials == 0)
		return 1;

	for (;;) {
		const cpu_specific_impl_t *impl = (const cpu_specific_impl_t *)p;

		if (impl->cpu_flags == (impl->cpu_flags & cpu_flags)) {
			double tbest = 1000000000000.0;
			size_t i;

			if (test_fn(impl) != 0) {
				printf("%s failed to validate\n", impl->desc);
				return 1;
			}

			for (i = 0; i < trials; i++) {
				double tavg;
				cycles_t t1 = LOCAL_PREFIX(cpucycles)();
				bench_fn(impl);
				t1 = LOCAL_PREFIX(cpucycles)() - t1;
				tavg = (double)t1 / units_count;
				if (tavg < tbest)
					tbest = tavg;
			}

			if (first_item) {
				printf("%u %s(s):\n", (unsigned int)units_count, units_desc);
				first_item = 0;
			}

			printf("  %12s, %8.2f cycles/%s\n", impl->desc, tbest, units_desc);
		}

		if (impl->cpu_flags == CPUID_GENERIC)
			return 0;
		p += impl_size;
	}
}
