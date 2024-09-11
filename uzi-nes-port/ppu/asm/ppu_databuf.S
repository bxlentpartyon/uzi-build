	.export _ppu_databuf
	.export _databuf_pos
	.export write_ppubuf

	.include "uzi_nes.inc"

.segment "DATA"
.define	PPU_MAX_WRITE	64	; Max number of bytes we can write to the PPU in one VBlank
.define PPU_BUF_BYTE	5	; Write buffer entry size for a single byte
.define PPU_BUF_SIZE	PPU_MAX_WRITE * PPU_BUF_BYTE
.define SCREENBUF_SIZE	896

_ppu_databuf:
	.align 16
	.res $200, $00	; this is more than we can actually write in one
			; vblank, but it simplifies the logic to assume that
			; we have 2 full pages, but know that we will hit a \0
			; before we get past 64-bytes-worth of buffer entries
_databuf_pos:
	.addr $0000
_databuf_maxbytes:
	.byte $40
tmp_num_bytes:
	.byte $00
; current address corresponding to our position in the data buffer
cur_databuf_ptr:
	.addr _ppu_databuf

; Write buffer layout
;  byte    0 = length of data (0 = no more data)
;  byte    1 = high byte of target PPU address
;  byte    2 = low byte of target PPU address
;  byte    3 = unused (for future expansion)
;  bytes 4-X = the data to draw (number of bytes determined by the length)
;
;  https://www.nesdev.org/wiki/The_frame_and_NMIs#Buffer_Formats

.segment "CODE"

.proc write_ppubuf
buf_write_loop_reset:
	; Y is the index in the current data buffer page
	ldy #0
buf_write_loop:
	lda cur_databuf_ptr, y
	tax

	; we've encountered the \0 entry in the buffer
	beq out_reset_counters 

	; store off number of bytes 
	stx tmp_num_bytes
	iny

	; load address to PPU
	lda cur_databuf_ptr, y
	tax
	iny
	lda cur_databuf_ptr, y
	iny
	sta PPU_ADDR
	stx PPU_ADDR

	; skip over unused byte for now
	iny

	ldx #0
write_bytes_loop:
	lda cur_databuf_ptr, y
	sta PPU_DATA

	inx
	; if x != tmp_num_bytes, loop
	cpx tmp_num_bytes
	bne write_bytes_loop
; end write_bytes_loop

	; increment cur_databuf_ptr if necessary
	iny				; iny from the byte read is delay until here
					; to avoid polluting the carry flag early
	bcc buf_write_loop
	inc cur_databuf_ptr+1
	jmp buf_write_loop_reset
; end buf_write_loop

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
