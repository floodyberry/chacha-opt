#ifndef SECURE_ZERO_H
#define SECURE_ZERO_H

#include "asmopt.h"

#if defined(__cplusplus)
extern "C" {
#endif

void secure_zero(uint8_t *p, size_t len);
int secure_zero_init(void);

#if defined(UTILITIES)
void secure_zero_fuzz(void);
void secure_zero_bench(void);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* SECURE_ZERO_H */

