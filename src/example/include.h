#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

int example(const int *arr, size_t count);

int example_init(void);
int example_test_all(void);

#if defined(__cplusplus)
}
#endif

#endif /* EXAMPLE_H */

