	.export		_bzero
	.import		popa

.segment "USR_ZEROPAGE": zeropage
cur_bzero_addr:	.addr $0000

.segment "CODE"
.proc	_bzero
	; store start address
	sta cur_bzero_addr
	stx cur_bzero_addr+1

	; put partial page length in y, number of pages in x
	jsr popa
	tay
	jsr popa
	tax

	; partial page counter is zero-indexed, and we need to ensure a write on
	; the zero-th byte, so decrement Y by one out the gate, i.e. if they
	; want to clear 256 bytes, we start clearing at offset 255 from the
	; start address
	dey
	lda #$00
clear_bytes_y:
	sta (cur_bzero_addr), y
	dey
	bpl clear_bytes_y

	; if X is negative (no more pages to write) bail out
	dex
	bmi out

	; we have another whole page to clear
	ldy #$ff
	lda #$00
	beq clear_bytes_y ; this will always branch, but saves a cycle over jmp

out:
	rts
.endproc
