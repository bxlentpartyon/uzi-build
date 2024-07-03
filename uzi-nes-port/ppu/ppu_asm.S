	.export		_screenbuf
	.export		_printbuf
	.export		_buffer_dump_pos
	.export		_ppu_dump_pos
	.export		_ppu_load_font
	.export		handle_vblank

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
font_load_ptr:
	.addr $0000

.segment "FONTDATA"
	fontdata: .incbin "font/newfont.chr"

.segment "CODE"

.proc ppu_reset
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
	lda screenbuf_advance
	clc
	adc _buffer_dump_pos
	sta _buffer_dump_pos
	bcc check_buffer_pos	; We only check the carry for the second operation.
				; If the second overflowed, so did the first.
	inc _ppu_dump_pos+1
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

; Heavily inspired by https://www.nesdev.org/wiki/CHR_ROM_vs._CHR_RAM#Switching_to_CHR_RAM
.proc _ppu_load_font
	ldx #$84		;
	stx MMC5_PRG_16x2_P0	; PRG-ROM low bank to 2 (font data)

	lda #<fontdata		;
	sta font_load_ptr	;
	lda #>fontdata		; load address of font data into zero page pointer
	sta font_load_ptr+1	;

	ldy #$00		;
	sty PPU_MASK		; disable rendering, just to be safe

	sty PPU_ADDR		;
	sty PPU_ADDR		; PPU_ADDR to $0000

	ldx #$10		; number of 256 byte pages to copy (4K)
font_load_loop:
	lda (font_load_ptr), y
	sta PPU_DATA
	iny
	bne font_load_loop
	inc font_load_ptr+1
	dex
	bne font_load_loop
	rts
.endproc

.proc ppu_spray
	ldy #$10
	sty PPU_ADDR
	ldy #$00
	sty PPU_ADDR

	ldy #$04
spray_loop:
	lda #$69
	sta PPU_DATA
	dey
	bne spray_loop

	rts
.endproc

.proc handle_vblank
	jsr dump_screenbuf
	jsr ppu_spray
	jsr ppu_reset

	rts
.endproc
