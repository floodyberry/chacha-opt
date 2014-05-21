#include "cpuid.h"
#include "cpuid_impl.inc"

static uint32_t cpuid_flags = CPUID_GENERIC;
static uint32_t cpuid_mask = ~(uint32_t)0;

uint32_t
cpuid(void) {
	if (cpuid_flags == CPUID_GENERIC)
		cpuid_flags = cpuid_impl();
	return cpuid_flags & cpuid_mask;
}

const void *
cpu_select(const void *impls, size_t impl_size, impl_test test_fn) {
	uint32_t cpu_flags = cpuid();
	const uint8_t *p = (const uint8_t *)impls;
	for (;;) {
		const cpu_specific_impl_t *impl = (const cpu_specific_impl_t *)p;
		if (impl->cpu_flags == (impl->cpu_flags & cpu_flags)) {
			if (test_fn(impl) == 0)
				return impl;
		}
		if (impl->cpu_flags == CPUID_GENERIC)
			return NULL;
		p += impl_size;
	}
}
