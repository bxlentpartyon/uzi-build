	.export _ppu_databuf
	.export _databuf_pos
	.export write_ppubuf
	.export _ppu_init_databuf, _ppu_readbuf, _ppu_readbuf_dirty

	.include "mmc5.inc"
	.include "ppu.inc"
	.include "uzi_nes.inc"

.segment "DATA"
_ppu_databuf:
	.align $100
	.res $200, $00	; this is more than we can actually write in one
			; vblank, but it simplifies the logic to assume that
			; we have 2 full pages, but know that we will hit a \0
			; before we get past 64-bytes-worth of buffer entries
_databuf_pos:
	.addr $0000
_ppu_readbuf:
	.align $100
	.res $40, $00
_ppu_readbuf_dirty:
	.byte $00
	
tmp_num_bytes:
	.byte $00
tmp_options_byte:
	.byte $00
databuf_ready:
	.byte $00

.segment "USR_ZEROPAGE": zeropage
; current address corresponding to our position in the data buffer
cur_databuf_ptr:
	.addr $0000

; Write buffer layout
;  byte    0 = length of data (0 = no more data)
;  byte    1 = high byte of target PPU address
;  byte    2 = low byte of target PPU address
;  byte    3 = 00xx 00RN
;	         ||   ||
;	         \|   |\- NULL descriptor (write len bytes of 0x00)
;		  |   \-- Read descriptor
;		  \------ PPU upper CHR bank bits (currently used for reads only)
;  byte	   4 = PPU lower CHR bank bits (currently used for reads only)
;  bytes 5-X = the data to draw (number of bytes determined by the length)
;
;  https://www.nesdev.org/wiki/The_frame_and_NMIs#Buffer_Formats

.segment "CODE"

.proc _ppu_init_databuf
	lda #<_ppu_databuf
	sta cur_databuf_ptr
	lda #>_ppu_databuf
	sta cur_databuf_ptr+1

	inc databuf_ready

	rts
.endproc

.proc write_ppubuf
	; don't do anything until we've actually initialized cur_databuf_ptr
	lda databuf_ready
	bne check_dirty_bit 
	jmp out

check_dirty_bit:
	lda _ppu_readbuf_dirty
	beq buf_write_loop_reset
	jmp out

buf_write_loop_reset:
	; Y is the index in the current data buffer page
	ldy #0
buf_write_loop:
	lda (cur_databuf_ptr), y

	; we've encountered the \0 entry in the buffer
	beq out_reset_counters 

	; store off number of bytes 
	tax
	stx tmp_num_bytes
	iny

	; load address to PPU
	lda (cur_databuf_ptr), y
	tax
	iny
	lda (cur_databuf_ptr), y
	iny
	sta PPU_ADDR
	stx PPU_ADDR

	; Handle options byte
	lda (cur_databuf_ptr), y
	iny
	sta tmp_options_byte

; first check for a read descriptor
	bit PPUBUF_READ_DESC_MASK	; check for a read descriptor
	beq check_null_desc

; handle read descriptor
	lsr	; shift off the low 4 bits of the flags to
	lsr	; get to the upper CHR bits
	lsr
	lsr
	sta MMC5_CHR_UPPER		; write upper CHR bank bits
	lda (cur_databuf_ptr), y	; for a read descriptor, the single data byte contains the low bits
	iny				; of the CHR bank number
	sta MMC5_CHR_4K_SEG1		; write lower CHR bank bits

	bit PPU_DATA	; throw away a PPU read to prime the data buffer
			; see https://www.nesdev.org/wiki/PPU_registers#The_PPUDATA_read_buffer_(post-fetch)
	ldx #0
read_desc_loop:
	lda PPU_DATA
	sta _ppu_readbuf, x
	inx
	cpx tmp_num_bytes	
	bne read_desc_loop
	jmp buf_write_loop
; end handle read descriptor

; handle NULL descriptor
check_null_desc:
	bit PPUBUF_NULL_DESC_MASK	; >tmp_options_byte is still in A
	beq out_options_byte

; handle NULL descriptor
	ldx tmp_num_bytes
	lda #$00
write_null_desc_loop:
	sta PPU_DATA
	dex
	bne write_null_desc_loop

	lda #1
	sta _ppu_readbuf_dirty

	jmp buf_write_loop
; end handle NULL descriptor

; handle write descriptor
out_options_byte:
	ldx #0
write_bytes_loop:
	lda (cur_databuf_ptr), y
	sta PPU_DATA

	inx
	; if x != tmp_num_bytes, loop
	cpx tmp_num_bytes
	bne write_bytes_loop
; end handle write descriptor

	; increment cur_databuf_ptr if necessary
	iny				; iny from the byte read is delay until here
					; to avoid polluting the carry flag early
	bne buf_write_loop
	inc cur_databuf_ptr+1
	jmp buf_write_loop_reset
; end buf_write_loop

out_reset_counters:
	; reset data buffer position to 0
	lda #0
	sta _databuf_pos
	sta _databuf_pos+1

	; write null byte to beginning of buffer to avoid re-writing a
	; stale buffer
	ldy #0
	sta (cur_databuf_ptr), y

	; reset data buffer pointer to beginning of data buffer
	lda #<_ppu_databuf
	sta cur_databuf_ptr
	lda #>_ppu_databuf
	sta cur_databuf_ptr+1
out:
	rts
.endproc
