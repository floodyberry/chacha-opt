SECTION_TEXT

GLOBAL secure_zero_x86
FN secure_zero_x86
pxor %xmm0, %xmm0
xorq %rax, %rax

testq $0xf, %rdi
jz secure_zero_x86_aligned

secure_zero_x86_alignloop:
andq %rsi, %rsi
jz secure_zero_x86_done
movb %al, 0(%rdi)
incq %rdi
decq %rsi
testq $0xf, %rdi
jnz secure_zero_x86_alignloop

secure_zero_x86_aligned:

cmpq $32, %rsi
jb secure_zero_x86_last31

secure_zero_x86_32:
subq $32, %rsi
movdqa %xmm0, 0(%rdi)
movdqa %xmm0, 16(%rdi)
addq $32, %rdi
cmpq $32, %rsi
jae secure_zero_x86_32

secure_zero_x86_last31:

testq $0x10, %rsi
jz secure_zero_x86_skip16
movaps %xmm0, 0(%rdi)
addq $16, %rdi
subq $16, %rsi
secure_zero_x86_skip16:

testq $0x8, %rsi
jz secure_zero_x86_skip8
movq %rax, 0(%rdi)
addq $8, %rdi
subq $8, %rsi
secure_zero_x86_skip8:

testq $0x4, %rsi
jz secure_zero_x86_skip4
movl %eax, 0(%rdi)
addq $4, %rdi
subq $4, %rsi
secure_zero_x86_skip4:

testq $0x2, %rsi
jz secure_zero_x86_skip2
movw %ax, 0(%rdi)
addq $2, %rdi
subq $2, %rsi
secure_zero_x86_skip2:

testq $0x1, %rsi
jz secure_zero_x86_skip1
movb %al, 0(%rdi)
secure_zero_x86_skip1:

secure_zero_x86_done:
ret
FN_END secure_zero_x86

