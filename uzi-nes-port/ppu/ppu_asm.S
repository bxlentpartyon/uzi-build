	.export		_ppu_reset
	.export		___dump_screenbuf
	.export		_screenbuf
	.export		_screenbuf_advance
	.export		_printbuf
	.export		_buffer_dump_pos
	.export		_ppu_dump_pos

	.include "uzi_nes.inc"

.segment "DATA"
_screenbuf:
	.res 896, $00
_screenbuf_advance:
	.byte $40
_printbuf:
	.res 896, $00

.segment "ZEROPAGE"
_buffer_dump_pos:
	.addr $0000
_ppu_dump_pos:
	.addr $0000

.segment "CHR"
	.incbin "font/newfont.chr"

.segment "CODE"

.proc _ppu_reset
	lda #$20	; reset ppu read address
	sta PPU_ADDR
	lda #$00
	sta PPU_ADDR

	sta PPU_SCROLL	; reset scrolling
	sta PPU_SCROLL

	ldx #$0a	; enable BG rendering
	stx PPU_MASK
	rts
.endproc

.proc ___dump_screenbuf
put_str:
	lda _ppu_dump_pos+1
	sta PPU_ADDR
	lda _ppu_dump_pos
	sta PPU_ADDR

	ldy #$00
loop:
	lda (_buffer_dump_pos), y
	sta PPU_DATA
	iny
	cpy _screenbuf_advance
	bne loop

put_str_out:
	rts

.endproc
