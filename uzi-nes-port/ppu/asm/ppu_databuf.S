	.export _ppu_databuf
	.export _databuf_pos

.segment "DATA"
.define	PPU_MAX_WRITE	64	; Max number of bytes we can write to the PPU in one VBlank
.define PPU_BUF_BYTE	5	; Write buffer entry size for a single byte
.define PPU_BUF_SIZE	PPU_MAX_WRITE * PPU_BUF_BYTE
.define SCREENBUF_SIZE	896
.define FIRST_VIS_ROW	$2020

_ppu_databuf:
	.align 16
	.res	PPU_MAX_WRITE * PPU_BUF_BYTE	; Worst-case scenario is 64 single-byte writes
_databuf_pos:
	.addr $0000
_databuf_maxbytes:
	.byte $40
_cur_screen_ptr:
	.addr FIRST_VIS_ROW

; Write buffer layout
;  byte    0 = length of data (0 = no more data)
;  byte    1 = high byte of target PPU address
;  byte    2 = low byte of target PPU address
;  byte    3 = unused (for future expansion)
;  bytes 4-X = the data to draw (number of bytes determined by the length)
;
;  https://www.nesdev.org/wiki/The_frame_and_NMIs#Buffer_Formats

.segment "ZEROPAGE"
; current address corresponding to our position in the data buffer
cur_databuf_ptr:
	.addr _ppu_databuf

.segment "CODE"

.proc write_ppubuf
buf_write_loop:
	ldx #0
	lda (cur_databuf_ptr), x

	beq loop_out

	;
	lda _cur_screen_ptr+1
	sta PPU_ADDR
	lda _cur_screen_ptr
	sta PPU_ADDR

out_reset_counters:
	; reset data buffer position to 0
	lda #0
	sta _databuf_pos

	; reset data buffer pointer to beginning of data buffer
	lda #<_ppu_databuf
	sta cur_databuf_ptr
	lda #>_ppu_databuf
	sta cur_databuf_ptr+1

	rts
.endproc
