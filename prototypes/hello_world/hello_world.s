.segment "HEADER"
	.byte $4e, $45, $53, $1a	; 0-3	NES<EOF>
	.byte 1				; 4	PRG-ROM Banks
	.byte 1				; 5	CHR-ROM Banks
	.byte $00				; 6	Horizontal mirroring, mapper 0
	.byte $08			; 7	Remaining 4 bits of mapper, iNES 2.0, NES system
	.byte 0				; 8	Even more mapper bits
	.byte 0				; 9	Repeat of PRG/CHR banks
	.byte 0				; 10	PRG-RAM/EEPROM size
	.byte 0				; 11	CHR-RAM size
	.byte 0				; 12	CPU timing (NTSC)
	.byte 0				; 13 	TV system (NTSC)
	.byte 0, 0			; 14-15 Misc ROM, expansion device

.segment "CODE"
reset:
	sei
	cld
	ldx #$40
	stx $4017
	ldx #$ff
	txs
	inx
	stx $2000
	stx $2001
	stx $4010

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

forever:
	jmp forever

nmi:
	rti

.segment "VECTORS"
	.word nmi
	.word reset
	.word 0

.segment "CHR"
	.incbin "empty.chr"
