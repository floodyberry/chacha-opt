#ifndef EXAMPLE_H
#define EXAMPLE_H

#include "asmopt.h"

#if defined(__cplusplus)
extern "C" {
#endif

int32_t example(const int32_t *arr, size_t count);
int example_init(void);

#if defined(UTILITIES)
void example_fuzzer(void);
void example_bench(void);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* EXAMPLE_H */

