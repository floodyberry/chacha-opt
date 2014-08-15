.arch armv7-a

SECTION_TEXT

GLOBAL secure_compare32_armv7
HIDDEN secure_compare32_armv7
FN secure_compare32_armv7
stmfd sp!, {r4-r10, lr}
ldr r2, [r0, #0]
ldr r4, [r0, #4]
ldr r6, [r0, #8]
ldr r8, [r0, #12]
ldr r3, [r1, #0]
ldr r5, [r1, #4]
ldr r7, [r1, #8]
ldr r9, [r1, #12]
eor r3, r2
eor r5, r4
ldr r2, [r0, #16]
eor r7, r6
ldr r4, [r0, #20]
eor r9, r8
ldr r6, [r0, #24]
orr r3, r5
ldr r8, [r0, #28]
orr r7, r9
ldr r5, [r1, #16]
orr r3, r7
ldr r7, [r1, #20]
ldr r9, [r1, #24]
ldr r10, [r1, #28]
eor r2, r5
eor r4, r7
eor r6, r9
eor r8, r10
eor r1, r1
orr r2, r4
orr r6, r8
sub r1, #1
orr r2, r3
orrs r0, r2, r6
orrne r0, r1
andeq r0, #0
ldmfd sp!, {r4-r10, pc}
FN_END secure_compare32_armv7

GLOBAL secure_compare16_armv7
HIDDEN secure_compare16_armv7
FN secure_compare16_armv7
stmfd sp!, {r4-r10, lr}
ldr r2, [r0, #0]
ldr r4, [r0, #4]
ldr r6, [r0, #8]
ldr r8, [r0, #12]
ldr r3, [r1, #0]
ldr r5, [r1, #4]
ldr r7, [r1, #8]
ldr r9, [r1, #12]
eor r2, r3
eor r4, r5
eor r6, r7
eor r8, r9
eor r1, r1
orr r2, r4
orr r6, r8
sub r1, #1
orrs r0, r2, r6
orrne r0, r1
andeq r0, #0
ldmfd sp!, {r4-r10, pc}
FN_END secure_compare16_armv7


GLOBAL secure_compare8_armv7
HIDDEN secure_compare8_armv7
FN secure_compare8_armv7
stmfd sp!, {r4-r5, lr}
ldr r2, [r0, #0]
ldr r4, [r0, #4]
ldr r3, [r1, #0]
ldr r5, [r1, #4]
eor r2, r3
eor r4, r5
eor r1, r1
orrs r0, r2, r4
sub r1, #1
orrne r0, r1
andeq r0, #0
ldmfd sp!, {r4-r5, pc}
FN_END secure_compare32_armv7

