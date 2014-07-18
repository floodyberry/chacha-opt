#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

LIB_PUBLIC unsigned char example(const unsigned char *arr, size_t count);
LIB_PUBLIC int example_init(void);

#if defined(UTILITIES)
void example_fuzz(void);
void example_bench(void);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* EXAMPLE_H */

