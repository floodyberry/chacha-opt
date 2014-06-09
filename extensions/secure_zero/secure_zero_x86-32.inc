SECTION_TEXT

GLOBAL secure_zero_x86
FN secure_zero_x86
movl 4(%esp), %edx
movl 8(%esp), %ecx
xorl %eax, %eax

cmpl $16, %ecx
jb secure_zero_x86_last15

secure_zero_x86_16:
subl $16, %ecx
movl %eax, 0(%edx)
movl %eax, 4(%edx)
movl %eax, 8(%edx)
movl %eax, 12(%edx)
addl $16, %edx
cmpl $16, %ecx
jae secure_zero_x86_16

secure_zero_x86_last15:
testl $0x8, %ecx
jz secure_zero_x86_skip8
movl %eax, 0(%edx)
movl %eax, 4(%edx)
addl $8, %edx
subl $8, %ecx
secure_zero_x86_skip8:

testl $0x4, %ecx
jz secure_zero_x86_skip4
movl %eax, 0(%edx)
addl $4, %edx
subl $4, %ecx
secure_zero_x86_skip4:

testl $0x2, %ecx
jz secure_zero_x86_skip2
movw %ax, 0(%edx)
addl $2, %edx
subl $2, %ecx
secure_zero_x86_skip2:

testl $0x1, %ecx
jz secure_zero_x86_skip1
movb %al, 0(%edx)
secure_zero_x86_skip1:

secure_zero_x86_done:
ret
FN_END secure_zero_x86

