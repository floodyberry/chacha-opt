#ifndef BENCH_H
#define BENCH_H

#include "asmopt.h"

typedef void (*bench_fn)(const void *impl);

/* a 32k, 64 byte aligned buffer to bench with */
uint8_t *bench_get_buffer(void);

void bench(const void *impls, size_t impl_size, bench_fn fn, size_t units_count, const char *units_desc, size_t trials);

#endif /* BENCH_H */

