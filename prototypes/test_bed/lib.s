.export ppu_on, put_str

.segment "CODE"

ppu_on:
	lda #0
	sta $2005
	sta $2005
	ldx #$00
	stx $2000
	ldx #$0a
	stx $2001
	rts

hello_str_end:	.byte 0
hello_str:	.byte $2f, $2c, $2c, $25, $28 ; backwards to work with dex
hello_str_len:	.byte 5

put_str:
	lda #$20
	sta $2006
	lda #$20
	sta $2006
	ldx hello_str_len
put_str_loop:
	lda hello_str_end, x
	sta $2007
	dex
	bne put_str_loop
	rts

