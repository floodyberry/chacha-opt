#include "cpuid.h"
#include "cpucycles.h"

#include "cpucycles_impl.inc"

cycles_t
LOCAL_PREFIX(cpucycles)(void) {
	return cpucycles_impl();
}
