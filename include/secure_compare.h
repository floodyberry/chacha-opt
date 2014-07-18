#ifndef SECURE_COMPARE_H
#define SECURE_COMPARE_H

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

LIB_PUBLIC int secure_compare8(const unsigned char *x, const unsigned char *y);
LIB_PUBLIC int secure_compare16(const unsigned char *x, const unsigned char *y);
LIB_PUBLIC int secure_compare32(const unsigned char *x, const unsigned char *y);
LIB_PUBLIC int secure_compare_init(void);

#if defined(UTILITIES)
void secure_compare_fuzz(void);
void secure_compare_bench(void);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* SECURE_COMPARE_H */

