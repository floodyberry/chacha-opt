#if defined(__GNUC__)
#include "gcc_driver.inc"
#else
;.if 0
%include "yasm_driver.inc"
;.endif
#endif

INCLUDE_IF_X86_64BIT "secure_compare/secure_compare_x86-64.inc"

INCLUDE_IF_SSE2_32BIT "secure_compare/secure_compare_sse2-32.inc"
INCLUDE_IF_X86_32BIT "secure_compare/secure_compare_x86-32.inc"
