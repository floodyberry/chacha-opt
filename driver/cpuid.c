#include "cpuid.h"

static unsigned long cpuid_flags = CPUID_GENERIC;

#if defined(ARCH_X86)
extern unsigned long cpuid_x86();
#endif

unsigned long
cpuid(void) {
	if (cpuid_flags == CPUID_GENERIC) {
#if defined(ARCH_X86)
		cpuid_flags = cpuid_x86();
#endif
	}
	return cpuid_flags;
}

const void *
cpu_select(const void *impls, size_t impl_size, impl_test test_fn) {
	unsigned long cpu_flags = cpuid();
	const char *p = (const char *)impls;
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

int
cpu_test_all(const void *impls, size_t impl_size, impl_test test_fn) {
	unsigned long cpu_flags = cpuid();
	const char *p = (const char *)impls;
	int res = 0;
	for (;;) {
		const cpu_specific_impl_t *impl = (const cpu_specific_impl_t *)p;
		if (impl->cpu_flags == (impl->cpu_flags & cpu_flags)) {
			if (test_fn(impl) != 0)
				res |= 1;
		}
		if (impl->cpu_flags == CPUID_GENERIC)
			return res;
		p += impl_size;
	}
}
