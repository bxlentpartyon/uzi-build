	.importzp	tmp4
	.import		callptr4
	.export		_trampoline, cur_code_bank

	.include "mmc5.inc"

.segment "DATA"
; bank value $00 means to load bank 0 as RAM, which is something we never
; do, since bank 0 is actually the initial RAM data.
cur_code_bank:
	.byte $00
trampoline_tmp:
	.byte $00

.segment "CODE"

.proc _trampoline
	; we can't use the regular CPU stack to store off A, because we need
	; to restore A right before the actual function call, instead of at
	; the very end of the trampoline
	sta trampoline_tmp

	; skip if trampolines are initialized
	lda cur_code_bank
	bne trampoline_main

trampoline_init:
	; Initialize trampoline by setting the code page and cur bank to the
	; same value.  This has to be done on the fly, as we can't be sure
	; which bank will get banked in first, and we need to avoid having
	; an invalid bank at the very bottom of the stack, as this will cause
	; unpredictable behavior when get back to the top layer of banked
	; calls.
	lda tmp4
	ora #$80
	sta MMC5_PRG_MODE3_P1
	sta cur_code_bank
	jmp call_target_noswitch

trampoline_main:
	; check if we need to switch pages
	lda tmp4
	ora #$80		; ROM banks always have bit 7 set
	cmp cur_code_bank
	beq call_target_noswitch

	; save off current code page - it's OK to use the stack for this one
	; because we'll pop the value back off before returning from here
	lda cur_code_bank
	pha

	; switch to new code page
	lda tmp4
	ora #$80
	sta MMC5_PRG_MODE3_P1
	sta cur_code_bank

	; restore A from stack before call
	lda trampoline_tmp
	jsr callptr4

	; restore previous code page
	sta trampoline_tmp
	pla
	sta MMC5_PRG_MODE3_P1
	sta cur_code_bank
	lda trampoline_tmp

	rts

call_target_noswitch:
	; restore A from stack before call
	lda trampoline_tmp
	jsr callptr4

	rts
.endproc
