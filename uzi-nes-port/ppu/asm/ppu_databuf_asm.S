	.export _ppu_databuf
	.export _databuf_pos
	.export process_ppubuf
	.export _ppu_init_databuf, _ppu_readbuf, _ppu_readbuf_dirty

	.include "mmc5.inc"
	.include "ppu.inc"
	.include "uzi_nes.inc"

.segment "DATA"
_ppu_databuf:
	.align $100
	.res $100, $00	; We use the Y register to track our progress through the
			; buffer during process_ppubuf.  We have to be sure that the
			; buffer is a maximum of one page long, because the loop in
			; that procedure can't handle an overflow in the Y register
			; at an arbitrary point in the loop
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

.segment "USR_ZEROPAGE": zeropage
; current address corresponding to our position in the data buffer
cur_databuf_ptr:
	.addr $0000

; PPU buffer layout
;
;  byte 0:	write descriptor:	length of data (0 = no more data)
;		 read descriotor:	number of bytes to read
;  byte 1:	high byte of target PPU address
;  byte	2:	low byte of target PPU address
;  byte	3:	00xx 0WRN
;		  ||  |||
;		  \|  ||\- NULL descriptor (write len bytes of 0x00)
;		   |  |\-- Read descriptor
;		   |  \--- Write descriptor
;		   \------ PPU upper CHR bank bits
; bytes 4-X:	   video write desc:	the data to draw (number of bytes determined by the length)
;		 non-video r/w desc:	PPU lower CHR bank bits (followed by data for writes)
;
;  Original idea borrowed from:
;  https://www.nesdev.org/wiki/The_frame_and_NMIs#Buffer_Formats

.segment "CODE"

.proc _ppu_init_databuf
	lda #<_ppu_databuf
	sta cur_databuf_ptr
	lda #>_ppu_databuf
	sta cur_databuf_ptr+1

	rts
.endproc

.proc process_ppubuf
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
	sta tmp_num_bytes
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

; first check for an r/w descriptor
	lda #PPUBUF_RW_DESC_MASK
	and tmp_options_byte
	beq check_null_desc

; handle bank switching
	lsr	; shift off the low 4 bits of the flags to
	lsr	; get to the upper CHR bits
	lsr
	lsr
	sta MMC5_CHR_UPPER		; write upper CHR bank bits
	lda (cur_databuf_ptr), y	; for an r/w descriptor, the first data byte contains the low bits
	iny				; of the CHR bank number
	sta MMC5_CHR_4K_SEG1		; write lower CHR bank bits

	; We don't have to explicitly check for a write descriptor here, since
	; we know one bit or the the other was set.
	lda #PPUBUF_READ_DESC_MASK
	and tmp_options_byte
	bne handle_read_desc

	; bank switch has been handled, so we can use the existing write
	; code from here
	jmp out_options_byte

handle_read_desc:
	bit PPU_DATA	; throw away a PPU read to prime the data buffer
			; see https://www.nesdev.org/wiki/PPU_registers#The_PPUDATA_read_buffer_(post-fetch)
	ldx #0
read_desc_loop:
	lda PPU_DATA
	sta _ppu_readbuf, x
	inx
	cpx tmp_num_bytes	
	bne read_desc_loop

	lda #1
	sta _ppu_readbuf_dirty

	jmp buf_write_loop
; end handle r/w descriptor

; check for NULL descriptor
check_null_desc:
	lda #PPUBUF_NULL_DESC_MASK
	and tmp_options_byte
	beq out_options_byte

; handle NULL descriptor
	ldx tmp_num_bytes
	lda #$00
write_null_desc_loop:
	sta PPU_DATA
	dex
	bne write_null_desc_loop

	jmp buf_write_loop
; end handle NULL descriptor

; handle write descriptor
out_options_byte:
	ldx #0
write_bytes_loop:
	lda (cur_databuf_ptr), y
	iny
	sta PPU_DATA

	inx
	; if x != tmp_num_bytes, loop
	cpx tmp_num_bytes
	bne write_bytes_loop
; end handle write descriptor

	jmp buf_write_loop
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
