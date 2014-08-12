.fpu neon

SECTION_TEXT

GLOBAL example_neon
HIDDEN example_neon
FN example_neon
stmfd sp!, {r4, lr}

eor r2, r2

ands r1, r1
beq Lexample_neon_done


Lexample_neon_alignloop:
tst r0, #0xf
beq Lexample_neon_aligned
ldrb r3, [r0], #1
add r2, r3
subs r1, #1
bne Lexample_neon_alignloop
b Lexample_neon_done

Lexample_neon_aligned:
bics r3, r1, #0xf
beq Lexample_neon_last15

veor q0, q0, q0

Lexample_neon_16bytes:
vld1.8 {q1}, [r0,:128]!
vadd.u8 q0, q0, q1
subs r3, #16
bne Lexample_neon_16bytes

vpadd.i8 d0, d0, d1
vpadd.i8 d0, d0, d0
vpadd.i8 d0, d0, d0
vpadd.i8 d0, d0, d0
vmov.u8 r3, d0[0]
add r2, r2, r3


Lexample_neon_last15:

ands r1, #15
beq Lexample_neon_done


Lexample_neon_final15:
ldrb r3, [r0], #1
add r2, r3
subs r1, #1
bne Lexample_neon_final15


Lexample_neon_done:
mov r0, r2
and r0, #0xff
ldmfd sp!, {r4, pc}
FN_END example_neon
