	.export		_ppu_load_font
	.export		handle_vblank
	.import		dump_screenbuf

	.include "uzi_nes.inc"
	.include "mmc5.inc"

.segment "ZEROPAGE"
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

; This is some experimental code for writing to PPU RAM.
; Keeping it here for now.
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
	jsr ppu_reset

	rts
.endproc
