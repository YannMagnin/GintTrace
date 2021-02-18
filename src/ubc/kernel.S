.section .gint.mapped, "ax"
.align 2

.global	_ubc_kernel_inth_reloc

/* ubc_kernel_inth_reloc(): Relocalised UBC handler */
_ubc_kernel_inth_reloc:
	! Generate "programme" context (used by the UBC handler)
	sts.l	pr, @-r15
	stc.l	spc, @-r15
	stc.l	ssr, @-r15
	sts.l	mach, @-r15
	sts.l	macl, @-r15
	stc.l	gbr, @-r15
	.word	0b0100111100110010	! SH4 intruction "stc.l sgr, @-r15"
	mov.l	r14, @-r15
	mov.l	r13, @-r15
	mov.l	r12, @-r15
	mov.l	r11, @-r15
	mov.l	r10, @-r15
	mov.l	r9, @-r15
	mov.l	r8, @-r15
	stc.l	R7_BANK, @-r15
	stc.l	R6_BANK, @-r15
	stc.l	R5_BANK, @-r15
	stc.l	R4_BANK, @-r15
	stc.l	R3_BANK, @-r15
	stc.l	R2_BANK, @-r15
	stc.l	R1_BANK, @-r15
	stc.l	R0_BANK, @-r15

	! Get which channel is trigger and clear interrupt flags.
	mov.l	ubc_ccmfr_reg, r0
	mov.l	@r0, r9
	mov	#0, r1
	mov.l	r1, @r0
	mov.l	icbi_addr, r2
	.word	0b0000001011100011	! SH4 instruction "icbi @r2"

	! Allow / unblock interrupt and switch register bank !
	stc	sr, r8
	mov	r8, r1
	mov.l	sr_mask, r0
	and	r0, r1
	ldc	r1, sr

	! Call high-level abstraction
	mov	r15, r4
	mov	r9, r5
	mov.l	ubc_handler, r0
	mov.l	@r0, r0
	tst	r0, r0
	bt 	ubc_kernel_inth_prologue
	jsr	@r0
	nop

ubc_kernel_inth_prologue:
	! force SR register
	ldc	r8, sr

	! Restore "program" context.
	ldc.l	@r15+, R0_BANK
	ldc.l	@r15+, R1_BANK
	ldc.l	@r15+, R2_BANK
	ldc.l	@r15+, R3_BANK
	ldc.l	@r15+, R4_BANK
	ldc.l	@r15+, R5_BANK
	ldc.l	@r15+, R6_BANK
	ldc.l	@r15+, R7_BANK
	mov.l	@r15+, r8
	mov.l	@r15+, r9
	mov.l	@r15+, r10
	mov.l	@r15+, r11
	mov.l	@r15+, r12
	mov.l	@r15+, r13
	mov.l	@r15+, r14
	.word	0b0100111100110110	! SH4 intruction "ldc.l @r15+, sgr"
	ldc.l	@r15+, gbr
	lds.l	@r15+, macl
	lds.l	@r15+, mach
	ldc.l	@r15+, ssr
	ldc.l	@r15+, spc
	lds.l	@r15+, pr

	! Clean exit.
	rte				! Interrupt Exit.
	nop				! (db) Safety first.

.align 4
ubc_handler:	.long _ubc_handler
ubc_ccmfr_reg:	.long 0xff200600
icbi_addr:	.long 0xa0000000
sr_mask:	.long ~0x300000f0



/* gint mecanism to store relocalised information */
.section .gint.mappedrel, "awx"
.align 2
.global	_ubc_kernel_inth
_ubc_kernel_inth:
	.long	_ubc_kernel_inth_reloc




.text
.global	_ubc_kernel_dbr_set
.global	_ubc_kernel_dbr_get
.global	_ubc_kernel_update

/* ubc_kernel_dbr_set(): switch the DBR address */
_ubc_kernel_dbr_set:
	.word 0b0000000011111010	! SH4 intruction "stc dbr, r0"
	.word 0b0100010011111010	! SH4 intruction "ldc r4, dbr"
	rts
	nop

/* ubc_kernel_dbr_get(): get the DBR address */
_ubc_kernel_dbr_get:
	.word 0b0000000011111010	! SH4 intruction "stc dbr, r0"
	rts
	nop

/* _ubc_kernel_update(): When the UBC is modified, we should involve icbi int*/
_ubc_kernel_update:
	mov	#0xa0, r4
	shll8	r4
	shll16	r4
	.word 0b0000010011100011	! SH4 intstruction "icbi @r4"
	rts
	nop
