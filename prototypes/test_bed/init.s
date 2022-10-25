.include "lib.inc"

.segment "HEADER"
	.byte $4e, $45, $53, $1a	; 0-3	NES<EOF>
	.byte 4				; 4	PRG-ROM Banks
	.byte 1				; 5	CHR-ROM Banks
	.byte $50			; 6	Horizontal mirroring, mapper 5
	.byte $08			; 7	Remaining 4 bits of mapper, iNES 2.0, NES system
	.byte 0				; 8	Even more mapper bits
	.byte 0				; 9	Repeat of PRG/CHR banks
	.byte 0				; 10	PRG-RAM/EEPROM size
	.byte 0				; 11	CHR-RAM size
	.byte 0				; 12	CPU timing (NTSC)
	.byte 0				; 13 	TV system (NTSC)
	.byte 0, 0			; 14-15 Misc ROM, expansion device

.segment "BANK00"
	.byte $69, $00

.segment "BANK01"
	.byte $69, $01

.segment "BANK02"
	.byte $69, $02

.segment "CODE"

.proc reset
	sei
	cld
	ldx #$40
	stx $4017

	ldx #$ff 	; set stack pointer to 0x01ff
	txs

	inx		; X = 0
	stx $2000	; disable vblank NMI
	stx $2001	; disable rendering
	stx $4010	; disable DMC IRQ

	bit $2002	; acknowledge stray vblank

vblankwait1:
	bit $2002
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

vblankwait2:
	bit $2002
	bne vblankwait2

set_palette:
	lda #$3f
	sta $2006
	lda #$01
	sta $2006
	lda #$30
	sta $2007

	jsr put_str
	jsr ppu_on

forever:
	jmp forever
.endproc

.proc nmi
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

	ldx #$00	;
	stx $5113	; PRG-RAM Page 0

	ldx #$82	;
	stx $5115	; PRG-ROM low bank to 1

	ldx #$06	;
	stx $5117	; PRG-ROM high bank to 3

	ldx #$03	;
	stx $5104	; Set ExRAM to read-only

	jmp reset
	.addr nmi, stub_entry, 0
.endscope
.endmacro

reset_stub "STUB00"
reset_stub "STUB01"
reset_stub "STUB02"
reset_stub "STUB03"

.segment "CHR"
	.incbin "tileset.chr"
