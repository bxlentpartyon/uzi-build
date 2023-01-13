.export _ppu_setup, _ppu_on, _reset_cursor, _put_chr

.segment "CODE"

_ppu_setup:
	pha
	lda #$3f
	sta $2006
	lda #$01
	sta $2006
	lda #$30
	sta $2007
	pla
	rts


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

