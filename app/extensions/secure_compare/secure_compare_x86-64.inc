SECTION_TEXT

GLOBAL secure_compare8_x86
HIDDEN secure_compare8_x86
FN_EXT secure_compare8_x86, 2, 0
movq 0(%rsi), %rax
movq $-1, %rcx
xorq 0(%rdi), %rax
cmovnz %rcx, %rax
ret
FN_END secure_compare8_x86

GLOBAL secure_compare16_x86
HIDDEN secure_compare16_x86
FN_EXT secure_compare16_x86, 2, 0
movq 0(%rsi), %rax
movq 8(%rsi), %rcx
movq $-1, %rdx
xorq 0(%rdi), %rax
xorq 8(%rdi), %rcx
orq %rcx, %rax
cmovnz %rdx, %rax
ret
FN_END secure_compare16_x86

GLOBAL secure_compare32_x86
HIDDEN secure_compare32_x86
FN_EXT secure_compare32_x86, 2, 0
movq 0(%rsi), %rax
movq 8(%rsi), %rdx
movq 16(%rsi), %rcx
movq 24(%rsi), %r8
xorq 0(%rdi), %rax
xorq 8(%rdi), %rdx
xorq 16(%rdi), %rcx
xorq 24(%rdi), %r8
movq $-1, %rsi
orq %rdx, %rax
orq %r8, %rcx
orq %rcx, %rax
cmovnz %rsi, %rax
ret
FN_END secure_compare32_x86

