	.importzp	sp
	.import 	_start_kernel, _stop_clock
	.import		nmi, irq
	.import		ppu_ctrl_val
	.export		stack_setup

	.include	"uzi_nes.inc"
	.include	"mmc5.inc"

.segment "HEADER"
	.byte $4e, $45, $53, $1a	; 0-3	NES<EOF>
	.byte 4				; 4	PRG-ROM Banks
	.byte 0				; 5	CHR-ROM Banks
	.byte $52			; 6	Mapper 5, battery-backed, mapper-controlled mirroring
	.byte $08			; 7	Remaining 4 bits of mapper, iNES 2.0, NES system
	.byte 0				; 8	Even more mapper bits
	.byte 0				; 9	Repeat of PRG/CHR banks
	.byte $aa			; 10	PRG-RAM/EEPROM size
	.byte $ff			; 11	CHR-RAM size (2M battery backed, 2M volatile)
	.byte 0				; 12	CPU timing (NTSC)
	.byte 0				; 13 	TV system (NTSC)
	.byte 0, 0			; 14-15 Misc ROM, expansion device

.segment "DATA"
stack_setup:
	.byte $00

.segment "CODE"

.proc reset
	sei
	cld

	jsr _stop_clock

	ldx #$ff 	; set stack pointer to 0x01ff
	txs

	; We need to explicitly write #$00 here, as ppu_ctrl_val isn't
	; initialized until after the copy_to_ram loop below.  Note that it's
	; not strictly necessary to initialize PPU_CTRL (at least in FCEUX).
	; I'm just doing it to be thorough.
	lda #$00
	sta PPU_CTRL	; disable vblank NMI

	inx		; X = 0
	stx PPU_MASK	; disable rendering
	stx DMC_CTRL	; disable DMC IRQ

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

	ldx #$0a		;
	stx MMC5_PRG_MODE1_P0	; PRG-RAM low bank to 10

	ldx #$00
clrmem2:
	lda #$00
	sta $8000, x
	sta $8100, x
	sta $8200, x
	sta $8300, x
	sta $8400, x
	sta $8500, x
	sta $8600, x
	sta $8700, x
	sta $8800, x
	sta $8900, x
	sta $8a00, x
	sta $8b00, x
	sta $8c00, x
	sta $8d00, x
	sta $8e00, x
	sta $8f00, x
	sta $9000, x
	sta $9100, x
	sta $9200, x
	sta $9300, x
	sta $9400, x
	sta $9500, x
	sta $9600, x
	sta $9700, x
	sta $9800, x
	sta $9900, x
	sta $9a00, x
	sta $9b00, x
	sta $9c00, x
	sta $9d00, x
	sta $9e00, x
	sta $9f00, x
	sta $a000, x
	sta $a100, x
	sta $a200, x
	sta $a300, x
	sta $a400, x
	sta $a500, x
	sta $a600, x
	sta $a700, x
	sta $a800, x
	sta $a900, x
	sta $aa00, x
	sta $ab00, x
	sta $ac00, x
	sta $ad00, x
	sta $ae00, x
	sta $af00, x
	sta $b000, x
	sta $b100, x
	sta $b200, x
	sta $b300, x
	sta $b400, x
	sta $b500, x
	sta $b600, x
	sta $b700, x
	sta $b800, x
	sta $b900, x
	sta $ba00, x
	sta $bb00, x
	sta $bc00, x
	sta $bd00, x
	sta $be00, x
	sta $bf00, x
	inx
	beq init_stack
	jmp clrmem2

init_stack:
	lda #$00
	ldx #$9f
	sta sp
	stx sp+1

	lda #01
	sta stack_setup

	cli

	jmp _start_kernel
.endproc

.macro reset_stub seg
.segment seg
.scope
stub_entry:
	sei
	ldx #$ff
	txs

	ldx #$01		;
	stx MMC5_CHR_MODE	; CHR bank mode 1 (4K banks)

	ldx #$02		;
	stx MMC5_CHR_UPPER	; Set high bits of CHR bank to 10

	ldx #$00
	stx MMC5_CHR_4K_SEG0	; Select first non-battery-backed bank for segment 0

	stx MMC5_CHR_UPPER	; Set high bits of CHR bank to 00
	stx MMC5_CHR_4K_SEG1	; Select CHR bank 0 for segment 1
	stx MMC5_NAMETABLE	; Set all nametables to 00

	stx MMC5_SCAN_IRQ_STATUS; Disable scanline IRQ

	ldx #$01		;
	stx MMC5_PRG_MODE	; PRG bank mode 01 (2 16K banks)

	ldx #$02		; Set PRG RAM protect 1
	stx MMC5_RAM_PROT1	;
	ldx #$01		; Set PRG RAM protect 2
	stx MMC5_RAM_PROT2	;

	ldx #$08		;
	stx MMC5_PRG_RAM_PM	; PRG-RAM Page 8 (first non-battery-backed page)

	ldx #$80		;
	stx MMC5_PRG_MODE1_P0	; PRG-ROM low bank to 0

	ldx #$04		;
	stx MMC5_PRG_ROM_PM	; PRG-ROM high bank to 2

	ldx #$02		;
	stx MMC5_EXRAM_MODE	; Set ExRAM to read-write

	ldx #$50		;
	stx MMC5_NAMETABLE	; Vertical mirroring

	jmp reset
	.addr nmi, stub_entry, irq
.endscope
.endmacro

reset_stub "STUB00"
reset_stub "STUB01"
reset_stub "STUB02"
reset_stub "STUB03"
