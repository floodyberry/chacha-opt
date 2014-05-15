#ifndef FUZZ_H
#define FUZZ_H

#include "driver.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef void (*impl_fuzz_setup)(uint8_t *in, size_t *in_bytes, size_t *out_bytes);
typedef size_t (*impl_fuzz)(const void *impl, const uint8_t *in, uint8_t *out);
typedef void (*impl_fuzz_print)(const void *impl, const uint8_t *in, const uint8_t *out, const uint8_t *generic_out);

void fuzz_init(void);
void fuzz_init_deterministic(void);
void fuzz_get_bytes(void *out, size_t len);
void fuzz(const void *impls, size_t impl_size, impl_fuzz_setup setup_fn, impl_fuzz fuzz_fn, impl_fuzz_print print_fn);

#if defined(__cplusplus)
}
#endif

#endif /* FUZZ_H */
