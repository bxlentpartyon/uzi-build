	.export		_screenbuf
	.export		_printbuf
	.export		buffer_dump_pos
	.export		_init_ppu, handle_vblank
	.export screenbuf_end

	.include "uzi_nes.inc"
	.include "mmc5.inc"

.segment "DATA"
_screenbuf:
	.res 896, $00
screenbuf_size:
	.word .sizeof(_screenbuf)
screenbuf_end:
	.addr (_screenbuf) + screenbuf_size
screenbuf_advance:
	.byte $40
ppu_first_vis_row:
	.addr $2020
_printbuf:
	.res 896, $00

.segment "ZEROPAGE"
buffer_dump_pos:
	.addr $0000
ppu_dump_pos:
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

.proc reset_buf_pos
	lda #>_screenbuf
	sta buffer_dump_pos+1
	lda #<_screenbuf
	sta buffer_dump_pos

	lda >ppu_first_vis_row
	sta ppu_dump_pos+1
	lda <ppu_first_vis_row
	sta ppu_dump_pos

	rts
.endproc

.proc handle_vblank
put_str:
	lda ppu_dump_pos+1
	sta PPU_ADDR
	lda ppu_dump_pos
	sta PPU_ADDR

	ldy #$00
loop:
	lda (buffer_dump_pos), y
	sta PPU_DATA
	iny
	cpy screenbuf_advance
	bne loop

; 16-bit add logic for ppu_dump_pos and buffer_dump_pos
	lda screenbuf_advance
	clc
	adc ppu_dump_pos
	clc
	adc buffer_dump_pos
	bcc check_buffer_end
	inc ppu_dump_pos+1
	inc buffer_dump_pos+1

check_buffer_end:
	lda screenbuf_size+1
	cmp >buffer_dump_pos
	bne spray_reset
	lda screenbuf_size
	cmp <buffer_dump_pos
	bne spray_reset

	jsr reset_buf_pos

spray_reset:
	jsr ppu_spray
	jsr ppu_reset

	rts
.endproc

; Heavily inspired by https://www.nesdev.org/wiki/CHR_ROM_vs._CHR_RAM#Switching_to_CHR_RAM
.proc ppu_load_font
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

.proc _init_ppu
	jsr ppu_load_font
	jsr reset_buf_pos
.endproc
