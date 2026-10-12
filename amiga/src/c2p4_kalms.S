/* Code Police — four-plane Kalms C2P for byte-per-pixel colours 0..15.
 *
 * Adapted from Mikael Kalms, c2p1x1_4_c5_bm.s (2016-09-01), Public Domain.
 * The upstream non-modulo transpose and pipelined stores are retained. This
 * version uses the GNU m68k syntax and a C ABI with explicit row/plane strides.
 *
 * Contract: source rows are a multiple of 32 bytes and contain only colours
 * 0..15; the wrapper keeps the portable C path for all other API inputs.
 * Arguments: src, width, height, source_skip, planes[4], destination_skip.
 */
 .text
 .balign 4
 .globl _agart_c2p4_kalms
_agart_c2p4_kalms:
 movem.l d2-d7/a2-a6,-(sp)
 move.l 56(sp),-(sp) /* private row counter */
 move.l 52(sp),a0
 move.l 68(sp),a1
 movem.l (a1),a3-a6
.Lkalms_row:
 move.l a0,a2
 adda.l 56(sp),a2 /* exclusive end of this source row */
 move.l (a0)+,d0
 move.l (a0)+,d2
 move.l (a0)+,d1
 move.l (a0)+,d3

 lsl.l #4,d0
 lsl.l #4,d1
 or.l d2,d0
 or.l d3,d1

 move.l (a0)+,d2
 move.l (a0)+,d6
 move.l (a0)+,d3
 move.l (a0)+,d7

 lsl.l #4,d2
 lsl.l #4,d3
 or.l d6,d2
 or.l d7,d3

 move.w d2,d6
 move.w d3,d7
 move.w d0,d2
 move.w d1,d3
 swap d2
 swap d3
 move.w d2,d0
 move.w d3,d1
 move.w d6,d2
 move.w d7,d3

 move.l #0x33333333,d4
 move.l d2,d6
 move.l d3,d7
 lsr.l #2,d6
 lsr.l #2,d7
 eor.l d0,d6
 eor.l d1,d7
 and.l d4,d6
 and.l d4,d7
 eor.l d6,d0
 eor.l d7,d1
 lsl.l #2,d6
 lsl.l #2,d7
 eor.l d6,d2
 eor.l d7,d3

 move.l #0x00ff00ff,d4
 move.l d1,d6
 move.l d3,d7
 lsr.l #8,d6
 lsr.l #8,d7
 eor.l d0,d6
 eor.l d2,d7
 bra .Lkalms_start

.Lkalms_block:
 move.l (a0)+,d0
 move.l (a0)+,d2
 move.l (a0)+,d1
 move.l (a0)+,d3
 move.l d7,(a3)+

 lsl.l #4,d0
 lsl.l #4,d1
 or.l d2,d0
 or.l d3,d1

 move.l (a0)+,d2
 move.l (a0)+,d6
 move.l (a0)+,d3
 move.l (a0)+,d7
 move.l d4,(a4)+

 lsl.l #4,d2
 lsl.l #4,d3
 or.l d6,d2
 or.l d7,d3

 move.w d2,d6
 move.w d3,d7
 move.w d0,d2
 move.w d1,d3
 swap d2
 swap d3
 move.w d2,d0
 move.w d3,d1
 move.w d6,d2
 move.w d7,d3
 move.l d5,(a5)+

 move.l #0x33333333,d4
 move.l d2,d6
 move.l d3,d7
 lsr.l #2,d6
 lsr.l #2,d7
 eor.l d0,d6
 eor.l d1,d7
 and.l d4,d6
 and.l d4,d7
 eor.l d6,d0
 eor.l d7,d1
 lsl.l #2,d6
 lsl.l #2,d7
 eor.l d6,d2
 eor.l d7,d3

 move.l #0x00ff00ff,d4
 move.l d1,d6
 move.l d3,d7
 lsr.l #8,d6
 lsr.l #8,d7
 eor.l d0,d6
 eor.l d2,d7
 move.l a1,(a6)+
.Lkalms_start:
 and.l d4,d6
 and.l d4,d7
 eor.l d6,d0
 eor.l d7,d2
 lsl.l #8,d6
 lsl.l #8,d7
 eor.l d6,d1
 eor.l d7,d3

 move.l #0x55555555,d4
 move.l d1,d5
 move.l d3,d7
 lsr.l #1,d5
 lsr.l #1,d7
 eor.l d0,d5
 eor.l d2,d7
 and.l d4,d5
 and.l d4,d7
 eor.l d5,d0
 eor.l d7,d2
 add.l d5,d5
 add.l d7,d7
 eor.l d1,d5
 eor.l d3,d7

 move.l d0,a1
 move.l d2,d4

 cmpa.l a0,a2
 bne .Lkalms_block

 move.l d7,(a3)+
 move.l d4,(a4)+
 move.l d5,(a5)+
 move.l a1,(a6)+
 adda.l 64(sp),a0
 move.l 72(sp),d0
 adda.l d0,a3
 adda.l d0,a4
 adda.l d0,a5
 adda.l d0,a6
 subq.l #1,(sp)
 bne.w .Lkalms_row
 addq.l #4,sp
 movem.l (sp)+,d2-d7/a2-a6
 rts
