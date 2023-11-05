	.importzp	sp
	.import 	_start_kernel
	.import		_handle_vblank, _handle_irq
	.export		___dump_screenbuf
	.export		_ppu_reset
	.export		_screenbuf
	.export		_screenbuf_advance
	.export		_printbuf
	.export		_buffer_dump_pos
	.export		_ppu_dump_pos
	.export		_ei
	.export		_di
	.export		_start_clock, _stop_clock
	.export		_apu_status_byte

	.include	"uzi_nes.inc"

.segment "HEADER"
	.byte $4e, $45, $53, $1a	; 0-3	NES<EOF>
	.byte 4				; 4	PRG-ROM Banks
	.byte 1				; 5	CHR-ROM Banks
	.byte $50			; 6	Horizontal mirroring, mapper 5
	.byte $08			; 7	Remaining 4 bits of mapper, iNES 2.0, NES system
	.byte 0				; 8	Even more mapper bits
	.byte 0				; 9	Repeat of PRG/CHR banks
	.byte $07				; 10	PRG-RAM/EEPROM size
	.byte 0				; 11	CHR-RAM size
	.byte 0				; 12	CPU timing (NTSC)
	.byte 0				; 13 	TV system (NTSC)
	.byte 0, 0			; 14-15 Misc ROM, expansion device

.segment "BANK00"
	.byte $00, $00

.segment "BANK01"
	.byte $00, $00

.segment "BANK02"
	.byte $00, $00

.segment "DATA"
_screenbuf:
	.res 896, $00
_screenbuf_advance:
	.byte $80
_printbuf:
	.res 896, $00

.segment "ZEROPAGE"
_buffer_dump_pos:
	.addr $0000
_ppu_dump_pos:
	.addr $0000
_apu_status_byte:
	.byte $00

.segment "CODE"

; enable interrupts
.proc _ei
	cli

	lda #%10000000
	sta PPU_CTRL	; enable vblank NMI

	rts
.endproc

; disable interrupts
.proc _di
	sei

	lda #$00
	sta PPU_CTRL	; disable vblank NMI

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

.proc _start_clock
	cli
	lda #%00000000
	sta APU_FRAME_COUNTER	; enable clock interrupt
	rts
.endproc

.proc _stop_clock
	sei
	lda #%01000000
	sta APU_FRAME_COUNTER	; disable clock interrupt
	rts
.endproc

.proc reset
	sei
	cld
	ldx #$40
	stx $4017

	ldx #$ff 	; set stack pointer to 0x01ff
	txs

	lda #%00000000
	sta PPU_CTRL	; disable vblank NMI

	jsr _stop_clock

	inx		; X = 0
	stx PPU_MASK	; disable rendering
	stx $4010	; disable DMC IRQ

	bit PPU_STATUS	; acknowledge stray vblank

vblankwait1:
	bit PPU_STATUS
	bpl vblankwait1

clrmem:
	lda #$00
	sta $0000, x
	sta $0100, x
	sta $0200, x
	sta $0300, x
	sta $0400, x
	sta $0500, x
	sta $0600, x
	sta $0700, x
	inx
	bne clrmem


	; copy data from bank 3 to ram
	ldx #$00
copy_to_ram:
	lda $8000, x
	sta $6000, x
	lda $8100, x
	sta $6100, x
	lda $8200, x
	sta $6200, x
	lda $8300, x
	sta $6300, x
	lda $8400, x
	sta $6400, x
	lda $8500, x
	sta $6500, x
	lda $8600, x
	sta $6600, x
	lda $8700, x
	sta $6700, x
	lda $8800, x
	sta $6800, x
	lda $8900, x
	sta $6900, x
	lda $8a00, x
	sta $6a00, x
	lda $8b00, x
	sta $6b00, x
	lda $8c00, x
	sta $6c00, x
	lda $8d00, x
	sta $6d00, x
	lda $8e00, x
	sta $6e00, x
	lda $8f00, x
	sta $6f00, x
	lda $9000, x
	sta $7000, x
	lda $9100, x
	sta $7100, x
	lda $9200, x
	sta $7200, x
	lda $9300, x
	sta $7300, x
	lda $9400, x
	sta $7400, x
	lda $9500, x
	sta $7500, x
	lda $9600, x
	sta $7600, x
	lda $9700, x
	sta $7700, x
	lda $9800, x
	sta $7800, x
	lda $9900, x
	sta $7900, x
	lda $9a00, x
	sta $7a00, x
	lda $9b00, x
	sta $7b00, x
	lda $9c00, x
	sta $7c00, x
	lda $9d00, x
	sta $7d00, x
	lda $9e00, x
	sta $7e00, x
	lda $9f00, x
	sta $7f00, x
	inx
	beq vblankwait2
	jmp copy_to_ram

vblankwait2:
	bit PPU_STATUS
	bne vblankwait2

set_palette:
	lda #$3f
	sta PPU_ADDR
	lda #$01
	sta PPU_ADDR
	lda #$30
	sta PPU_DATA

	lda #$00
	ldx #$08
	sta sp
	stx sp+1

	cli

	jmp _start_kernel
.endproc

.proc nmi
	pha	; save registers
	tya
	pha
	txa
	pha

	jsr _handle_vblank

	pla	; restore registers
	tax
	pla
	tay
	pla

	rti
.endproc

.proc irq
	pha	; save registers
	tya
	pha
	txa
	pha

	lda APU_STATUS
	sta _apu_status_byte

	jsr _handle_irq

	pla	; restore registers
	tax
	pla
	tay
	pla

	rti
.endproc

.macro reset_stub seg
.segment seg
.scope
stub_entry:
	sei
	ldx #$ff
	txs

	ldx #$00
	stx $5101	; CHR bank mode 00
	stx $5130	; Set high bits of CHR bank to 00
	stx $5127	; Select CHR bank 0

	stx $5105	; Set all nametables to 00

	ldx #$01	;
	stx $5100	; PRG bank mode 01 (2 16K banks)

	ldx #$02	; Set PRG RAM protect 1
	stx $5102
	ldx #$01	; Set PRG RAM protect 2
	stx $5103

	ldx #$01	;
	stx $5113	; PRG-RAM Page 0

	ldx #$82	;
	stx $5115	; PRG-ROM low bank to 1

	ldx #$06	;
	stx $5117	; PRG-ROM high bank to 3

	ldx #$03	;
	stx $5104	; Set ExRAM to read-only

	jmp reset
	.addr nmi, stub_entry, irq
.endscope
.endmacro

reset_stub "STUB00"
reset_stub "STUB01"
reset_stub "STUB02"
reset_stub "STUB03"

.segment "CHR"
	.incbin "font/newfont.chr"
