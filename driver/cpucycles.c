#include "cpuid.h"
#include "cpucycles.h"

#include "cpucycles_impl.inc"

cycles_t
cpucycles(void) {
	return cpucycles_impl();
}
