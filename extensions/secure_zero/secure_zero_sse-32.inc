SECTION_TEXT

GLOBAL secure_zero_sse
FN secure_zero_sse
movl 4(%esp), %edx
movl 8(%esp), %ecx
xorps %xmm0, %xmm0
xorl %eax, %eax

testl $0xf, %edx
jz secure_zero_sse_aligned

secure_zero_sse_alignloop:
andl %ecx, %ecx
jz secure_zero_sse_done
movb %al, 0(%edx)
incl %edx
decl %ecx
testl $0xf, %edx
jnz secure_zero_sse_alignloop

secure_zero_sse_aligned:

cmpl $32, %ecx
jb secure_zero_sse_last31

secure_zero_sse_32:
subl $32, %ecx
movaps %xmm0, 0(%edx)
movaps %xmm0, 16(%edx)
addl $32, %edx
cmpl $32, %ecx
jae secure_zero_sse_32

secure_zero_sse_last31:

testl $0x10, %ecx
jz secure_zero_sse_skip16
movaps %xmm0, 0(%edx)
addl $16, %edx
subl $16, %ecx
secure_zero_sse_skip16:

testl $0x8, %ecx
jz secure_zero_sse_skip8
movl %eax, 0(%edx)
movl %eax, 4(%edx)
addl $8, %edx
subl $8, %ecx
secure_zero_sse_skip8:

testl $0x4, %ecx
jz secure_zero_sse_skip4
movl %eax, 0(%edx)
addl $4, %edx
subl $4, %ecx
secure_zero_sse_skip4:

testl $0x2, %ecx
jz secure_zero_sse_skip2
movw %ax, 0(%edx)
addl $2, %edx
subl $2, %ecx
secure_zero_sse_skip2:

testl $0x1, %ecx
jz secure_zero_sse_skip1
movb %al, 0(%edx)
secure_zero_sse_skip1:

secure_zero_sse_done:
ret
FN_END secure_zero_sse

