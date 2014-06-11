SECTION_TEXT

GLOBAL secure_compare8_x86
FN_EXT secure_compare8_x86, 2, 0
movq 0(%rsi), %rax
movq 0(%rdi), %rdx
movq $-1, %rcx
xorq %rdx, %rax
cmovnz %rcx, %rax
ret
FN_END secure_compare8_x86

GLOBAL secure_compare16_x86
FN_EXT secure_compare16_x86, 2, 0
movq 0(%rsi), %rax
movq 8(%rsi), %rcx
movq 0(%rdi), %rdx
movq 8(%rdi), %r8
xorq %rdx, %rax
xorq %r8, %rcx
movq $-1, %rdx
orq %rcx, %rax
cmovnz %rdx, %rax
ret
FN_END secure_compare16_x86

GLOBAL secure_compare32_x86
FN_EXT secure_compare32_x86, 2, 0
movq 0(%rsi), %rax
movq 8(%rsi), %rcx
movq 0(%rdi), %rdx
movq 8(%rdi), %r8
xorq %rdx, %rax
xorq %r8, %rcx
orq %rcx, %rax
movq 16(%rsi), %r9
movq 24(%rsi), %rcx
movq 16(%rdi), %rdx
movq 24(%rdi), %r8
xorq %rdx, %r9
xorq %r8, %rcx
orq %rcx, %r9
movq $-1, %rdx
orq %r9, %rax
cmovnz %rdx, %rax
ret
FN_END secure_compare32_x86

