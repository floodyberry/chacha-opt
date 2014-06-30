#ifndef EXAMPLE_H
#define EXAMPLE_H

#include "asmopt_example.h"

#if defined(__cplusplus)
extern "C" {
#endif

LIB_PUBLIC int32_t example(const int32_t *arr, size_t count);
LIB_PUBLIC int example_init(void);

#if defined(UTILITIES)
void example_fuzz(void);
void example_bench(void);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* EXAMPLE_H */

