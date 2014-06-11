#ifndef SECURE_COMPARE_H
#define SECURE_COMPARE_H

#include "asmopt.h"

#if defined(__cplusplus)
extern "C" {
#endif

int secure_compare8(const uint8_t *x, const uint8_t *y);
int secure_compare16(const uint8_t *x, const uint8_t *y);
int secure_compare32(const uint8_t *x, const uint8_t *y);
int secure_compare_init(void);

#if defined(UTILITIES)
void secure_compare_fuzz(void);
void secure_compare_bench(void);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* SECURE_COMPARE_H */

