#include "cpucycles.h"
#include "cpuid.h"

#include "cpucycles_impl.inc"

cycles_t
cpucycles(void) {
	return cpucycles_impl();
}

/*
void
cpucycles_bench(void *impl, size_t impl_len, size_t ) {
}
*/