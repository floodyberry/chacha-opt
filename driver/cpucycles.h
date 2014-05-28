#ifndef CPUCYCLES_H
#define CPUCYCLES_H

#include "driver.h"

#if defined(HAVE_INT64)
typedef uint64_t cycles_t;
#else
typedef uint32_t cycles_t;
#endif

cycles_t cpucycles(void);

#if defined(UTILITIES)
typedef void (*bench_fn)(const void *impl);

/* a 32k, 64 byte aligned buffer to bench with */
uint8_t *bench_get_buffer(void);

void bench(const void *impls, size_t impl_size, bench_fn fn, size_t units_count, const char *units_desc, size_t trials);
#endif /* defined(UTILITIES) */

#endif /* CPUCYCLES_H */

