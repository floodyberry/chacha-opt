#if defined(__GNUC__)
#include "gcc_driver.inc"
#else
;.if 0
%include "yasm_driver.inc"
;.endif
#endif

INCLUDE_IF_X86_64BIT "secure_zero/secure_zero_x86-64.inc"

INCLUDE_IF_SSE_32BIT "secure_zero/secure_zero_sse-32.inc"
INCLUDE_IF_X86_32BIT "secure_zero/secure_zero_x86-32.inc"
