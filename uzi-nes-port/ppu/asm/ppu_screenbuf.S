	.export		_screenbuf
	.export		_printbuf
	.export		_buffer_dump_pos
	.export		_ppu_dump_pos
	.export		dump_screenbuf

	.include "uzi_nes.inc"
	.include "mmc5.inc"

.segment "DATA"
.define SCREENBUF_SIZE	896
_screenbuf:
	.res SCREENBUF_SIZE, $00
screenbuf_end:
	.word _screenbuf + SCREENBUF_SIZE
screenbuf_advance:
	.byte $40
_printbuf:
	.res SCREENBUF_SIZE, $00
ppu_dump_start:
	.addr $2020

.segment "ZEROPAGE"
_buffer_dump_pos:
	.addr $0000
_ppu_dump_pos:
	.addr $0000

.segment "CODE"

.proc dump_screenbuf
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
	cpy screenbuf_advance
	bne loop

; 16-bit add logic for ppu_dump_pos and buffer_dump_pos
	lda screenbuf_advance
	clc
	adc _ppu_dump_pos
	sta _ppu_dump_pos
	bcc skip_ppu_upper_add
	inc _ppu_dump_pos+1

skip_ppu_upper_add:
	lda screenbuf_advance
	clc
	adc _buffer_dump_pos
	sta _buffer_dump_pos
	bcc check_buffer_pos
	inc _buffer_dump_pos+1

; check if buffer_dump_pos == screenbuf_end
check_buffer_pos:
	lda _buffer_dump_pos
	cmp #<screenbuf_end
	bcc out
	lda _buffer_dump_pos+1
	cmp #>screenbuf_end
	bcc out

	lda #<_screenbuf
	sta _buffer_dump_pos
	lda #>_screenbuf
	sta _buffer_dump_pos+1
	lda ppu_dump_start
	sta _ppu_dump_pos
	lda ppu_dump_start+1
	sta _ppu_dump_pos+1

out:
	rts
.endproc
