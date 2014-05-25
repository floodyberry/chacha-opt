#include "cpucycles.h"
#include "cpuid.h"

#include "cpucycles_impl.inc"

cycles_t
cpucycles(void) {
	return cpucycles_impl();
}

#if defined(UTILITIES)

#include <stdio.h>

/* a 64k, 64 byte aligned buffer to bench with */
uint8_t *bench_get_buffer(void) {
	static uint8_t buffer[0x10000 + 0x40 + 0x40];
	uint8_t *p = buffer;
	p += 0x3f;
	p -= (size_t)p & 0x3f;
	return p;
}

void
bench(const void *impls, size_t impl_size, bench_fn fn, const char *units_desc, size_t trials) {
	uint32_t cpu_flags = cpuid();
	const uint8_t *p = (const uint8_t *)impls;
	int first_item = 1;

	if (trials == 0)
		return;

	for (;;) {
		const cpu_specific_impl_t *impl = (const cpu_specific_impl_t *)p;

		if (impl->cpu_flags == (impl->cpu_flags & cpu_flags)) {
			double tbest = 1000000000000.0;
			size_t units = 1, i;

			for (i = 0; i < trials; i++) {
				double tavg;
				cycles_t t1 = cpucycles();
				units = fn(impl);
				t1 = cpucycles() - t1;
				tavg = (double)t1 / units;
				if (tavg < tbest)
					tbest = tavg;
			}

			if (first_item) {
				printf("%u %s(s):\n", (unsigned int)units, units_desc);
				first_item = 0;
			}

			printf("  %12s, %8.2f cycles/%s\n", impl->desc, tbest, units_desc);
		}

		if (impl->cpu_flags == CPUID_GENERIC)
			return;
		p += impl_size;
	}
}

#endif /* defined(UTILITIES) */
