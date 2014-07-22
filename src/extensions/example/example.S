#if defined(__GNUC__)
#include "gcc_driver.inc"
#else
;.if 0
%include "yasm_driver.inc"
;.endif
#endif

INCLUDE_IF_AVX_64BIT "example/example_avx-64.inc"
INCLUDE_IF_SSE2_64BIT "example/example_sse2-64.inc"

INCLUDE_IF_SSE2_32BIT "example/example_sse2-32.inc"
INCLUDE_IF_X86_32BIT "example/example_x86-32.inc"
