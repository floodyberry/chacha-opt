SECTION_TEXT

GLOBAL secure_compare8_x86
HIDDEN secure_compare8_x86
FN secure_compare8_x86
pushl %esi
movl 8(%esp), %edx
movl 12(%esp), %ecx
xorl %eax, %eax
movl 0(%edx), %esi
xorl 0(%ecx), %esi
orl %esi, %eax
movl 4(%edx), %esi
xorl 4(%ecx), %esi
orl %esi, %eax
movl %eax, %edx
shrl $16, %eax
orl %edx, %eax
orb %ah, %al
andl $0xff, %eax
decl %eax
shrl $8, %eax
andl $1, %eax
decl %eax
popl %esi
ret
FN_END secure_compare8_x86

GLOBAL secure_compare16_x86
HIDDEN secure_compare16_x86
FN secure_compare16_x86
pushl %esi
movl 8(%esp), %edx
movl 12(%esp), %ecx
xorl %eax, %eax
movl 0(%edx), %esi
xorl 0(%ecx), %esi
orl %esi, %eax
movl 4(%edx), %esi
xorl 4(%ecx), %esi
orl %esi, %eax
movl 8(%edx), %esi
xorl 8(%ecx), %esi
orl %esi, %eax
movl 12(%edx), %esi
xorl 12(%ecx), %esi
orl %esi, %eax
movl %eax, %edx
shrl $16, %eax
orl %edx, %eax
orb %ah, %al
andl $0xff, %eax
decl %eax
shrl $8, %eax
andl $1, %eax
decl %eax
popl %esi
ret
FN_END secure_compare16_x86

GLOBAL secure_compare32_x86
HIDDEN secure_compare32_x86
FN secure_compare32_x86
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
orl %edi, %eax
movl 8(%edx), %esi
movl 12(%edx), %edi
xorl 8(%ecx), %esi
xorl 12(%ecx), %edi
orl %esi, %eax
orl %edi, %eax
movl 16(%edx), %esi
movl 20(%edx), %edi
xorl 16(%ecx), %esi
xorl 20(%ecx), %edi
orl %esi, %eax
orl %edi, %eax
movl 24(%edx), %esi
movl 28(%edx), %edi
xorl 24(%ecx), %esi
xorl 28(%ecx), %edi
orl %esi, %eax
orl %edi, %eax
movl %eax, %edx
shrl $16, %eax
orl %edx, %eax
orb %ah, %al
andl $0xff, %eax
decl %eax
shrl $8, %eax
andl $1, %eax
decl %eax
popl %edi
popl %esi
ret
FN_END secure_compare32_x86

