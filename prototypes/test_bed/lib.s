.export _ppu_on, _reset_cursor, _put_chr

.segment "CODE"

_ppu_on:
	pha
	txa
	pha

	lda #0
	sta $2005
	sta $2005
	ldx #$00
	stx $2000
	ldx #$0a
	stx $2001

	pla
	tax
	pla

	rts

hello_str_end:	.byte 0
hello_str:	.byte $2f, $2c, $2c, $25, $28 ; backwards to work with dex
hello_str_len:	.byte 5

_reset_cursor:
	pha
	lda #$20
	sta $2006
	lda #$20
	sta $2006
	pla
	rts

_put_chr:
	sta $2007
	rts

