SECTION_TEXT

GLOBAL_HIDDEN_FN secure_compare8_sse2
pushl %esi
pushl %edi
movl 12(%esp), %edx
movl 16(%esp), %ecx
xorl %eax, %eax
movl 0(%edx), %esi
movl 4(%edx), %edi
xorl 0(%ecx), %esi
xorl 4(%ecx), %edi
orl %esi, %eax
movl $-1, %edx
orl %edi, %eax
cmovnz %edx, %eax
popl %edi
popl %esi
ret
FN_END secure_compare8_sse2

GLOBAL_HIDDEN_FN secure_compare16_sse2
movl 4(%esp), %edx
movl 8(%esp), %ecx
movdqu 0(%edx), %xmm0
movdqu 0(%ecx), %xmm1
pxor %xmm1, %xmm0
movl $-1, %edx
pshufd $0xee, %xmm0, %xmm2
por %xmm2, %xmm0
pshufd $0x55, %xmm0, %xmm1
por %xmm1, %xmm0
movd %xmm0, %eax
andl %eax, %eax
cmovnz %edx, %eax
ret
FN_END secure_compare16_sse2

GLOBAL_HIDDEN_FN secure_compare32_sse2
movl 4(%esp), %edx
movl 8(%esp), %ecx
movdqu 0(%edx), %xmm0
movdqu 16(%edx), %xmm1
movdqu 0(%ecx), %xmm2
movdqu 16(%ecx), %xmm3
pxor %xmm2, %xmm0
pxor %xmm3, %xmm1
por %xmm1, %xmm0
movl $-1, %edx
pshufd $0xee, %xmm0, %xmm2
por %xmm2, %xmm0
pshufd $0x55, %xmm0, %xmm1
por %xmm1, %xmm0
movd %xmm0, %eax
andl %eax, %eax
cmovnz %edx, %eax
ret
FN_END secure_compare32_sse2

