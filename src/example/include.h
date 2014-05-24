#ifndef EXAMPLE_H
#define EXAMPLE_H

#include "driver.h"

#if defined(__cplusplus)
extern "C" {
#endif

int32_t example(const int32_t *arr, size_t count);
int example_init(void);
void example_fuzzer(void);
void example_bench(void);

#if defined(__cplusplus)
}
#endif

#endif /* EXAMPLE_H */

