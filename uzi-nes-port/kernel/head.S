	.importzp	sp
	.import 	_start_kernel, _stop_clock
	.import		nmi, irq
	.import		ppu_ctrl_val
	.export		stack_setup

	.include	"uzi_nes.inc"
	.include	"mmc5.inc"

.segment "HEADER"
	.byte $4e, $45, $53, $1a	; 0-3	NES<EOF>
	.byte 3				; 4	PRG-ROM Banks
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

.segment "BANK00"
	.byte $00, $00

.segment "BANK01"
	.byte $00, $00

.segment "DATA"
stack_setup:
	.byte $00

.segment "CODE"

.proc reset
	sei
	cld
	ldx #$40
	stx $4017

	ldx #$ff 	; set stack pointer to 0x01ff
	txs

	lda ppu_ctrl_val
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
	lda $e000, x
	sta $6000, x
	lda $e100, x
	sta $6100, x
	lda $e200, x
	sta $6200, x
	lda $e300, x
	sta $6300, x
	lda $e400, x
	sta $6400, x
	lda $e500, x
	sta $6500, x
	lda $e600, x
	sta $6600, x
	lda $e700, x
	sta $6700, x
	lda $e800, x
	sta $6800, x
	lda $e900, x
	sta $6900, x
	lda $ea00, x
	sta $6a00, x
	lda $eb00, x
	sta $6b00, x
	lda $ec00, x
	sta $6c00, x
	lda $ed00, x
	sta $6d00, x
	lda $ee00, x
	sta $6e00, x
	lda $ef00, x
	sta $6f00, x
	lda $f000, x
	sta $7000, x
	lda $f100, x
	sta $7100, x
	lda $f200, x
	sta $7200, x
	lda $f300, x
	sta $7300, x
	lda $f400, x
	sta $7400, x
	lda $f500, x
	sta $7500, x
	lda $f600, x
	sta $7600, x
	lda $f700, x
	sta $7700, x
	lda $f800, x
	sta $7800, x
	lda $f900, x
	sta $7900, x
	lda $fa00, x
	sta $7a00, x
	lda $fb00, x
	sta $7b00, x
	lda $fc00, x
	sta $7c00, x
	lda $fd00, x
	sta $7d00, x
	lda $fe00, x
	sta $7e00, x
	lda $ff00, x
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

	ldx #$02		;
	stx MMC5_PRG_MODE	; PRG bank mode 02 (1 16K bank and 2 8K banks)

	ldx #$02		; Set PRG RAM protect 1
	stx MMC5_RAM_PROT1	;
	ldx #$01		; Set PRG RAM protect 2
	stx MMC5_RAM_PROT2	;

	ldx #$08		;
	stx MMC5_PRG_RAM_PM	; PRG-RAM Page 8 (first non-battery-backed page)

	ldx #$80		;
	stx MMC5_PRG_MODE2_P0	; PRG-ROM page 0 to bank 0

	ldx #$82		;
	stx MMC5_PRG_MODE2_P1	; PRG-ROM page 1 to bank 1

	ldx #$04		;
	stx MMC5_PRG_ROM_PM	; PRG-ROM permanent page to bank 4

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
