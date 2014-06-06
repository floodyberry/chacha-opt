#ifndef BENCH_H
#define BENCH_H

#include "asmopt.h"

typedef void (*impl_bench)(const void *impl);

/* a 32k, 64 byte aligned buffer to bench with */
uint8_t *bench_get_buffer(void);

void bench(const void *impls, size_t impl_size, impl_bench fn, size_t units_count, const char *units_desc, size_t trials);

#endif /* BENCH_H */

