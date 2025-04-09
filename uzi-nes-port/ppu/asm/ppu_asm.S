	.export		_ppu_load_font
	.export		handle_vblank, ppu_ctrl_val
	.export		_wait_frame, _frame_wait_sleep, _y_scroll_coarse, _y_scroll_fine
	.import		dump_screenbuf
	.import		process_ppubuf
	.export		_ppu_locked

	.include "uzi_nes.inc"
	.include "mmc5.inc"

.segment "USR_ZEROPAGE": zeropage
font_load_ptr:
	.addr $0000

.segment "FONTDATA"
	; we need room for the reset stub at the end of the page, and the font
	; data doesn't take up nearly the full 8K, so include all but the last
	; 256 bytes
	fontdata: .incbin "font/newfont.chr", 0, $1f00

.segment "DATA"
_frame_wait_sleep:
	.byte $00
ppu_ctrl_val:
	.byte $00
_y_scroll_coarse:
	.byte $00
_y_scroll_fine:
	; there's apparently a 1-pixel offset that I don't understand...
	.addr $0001
_ppu_locked:
	.byte $01	; starts out locked, must be unlocked by init code

.segment "CODE"

.proc ppu_reset
; A few VBlank cycles could probably be saved here, as I don't think it's really
; necessary to write this stuff every VBlank.
	lda #$20	; reset ppu read address
	sta PPU_ADDR
	lda #$00
	sta PPU_ADDR

	lda ppu_ctrl_val
	ldx _y_scroll_coarse
	beq clear_high_bit
set_high_bit:
	ora #%00000010
	jmp set_ppu_ctrl
clear_high_bit:
	and #%11111101
set_ppu_ctrl:
	sta ppu_ctrl_val
	sta PPU_CTRL	; store off "buffered" changes to PPU_CTRL (i.e. high scroll bits)

	bit PPU_STATUS		; reset w flag before scroll register writes
	lda #0			;
	sta PPU_SCROLL		; set X scroll to 0
	lda _y_scroll_fine	;
	sta PPU_SCROLL		; write Y scroll value

	ldx #$0a	; enable BG rendering
	stx PPU_MASK
	rts
.endproc

; Heavily inspired by https://www.nesdev.org/wiki/CHR_ROM_vs._CHR_RAM#Switching_to_CHR_RAM
.proc _ppu_load_font
	ldx #KERNEL_FONTDATA_ROM
	stx MMC5_PRG_MODE3_P0	; PRG-ROM page 1 to bank 1 (font data)

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
	lda _ppu_locked
	bne vblank_out

	jsr process_ppubuf
	jsr ppu_reset

	; wake up processes sleeping in wait_frame
	lda #0
	sta _frame_wait_sleep

vblank_out:
	rts
.endproc

.proc _wait_frame
	lda #1
	sta _frame_wait_sleep

wait_frame_loop:
	lda _frame_wait_sleep
	bne wait_frame_loop

	rts
.endproc
