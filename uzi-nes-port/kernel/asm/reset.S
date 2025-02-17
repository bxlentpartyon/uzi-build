	.import		nmi, irq, reset

	.include	"mmc5.inc"

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

	ldx #$03		;
	stx MMC5_PRG_MODE	; PRG bank mode 03 (5 8K banks)

	ldx #$02		; Set PRG RAM protect 1
	stx MMC5_RAM_PROT1	;
	ldx #$01		; Set PRG RAM protect 2
	stx MMC5_RAM_PROT2	;

	ldx #$08		;
	stx MMC5_PRG_RAM_PM	; PRG-RAM permanent page to bank 8 (first non-battery-backed bank)

	ldx #$80		;
	stx MMC5_PRG_MODE3_P0	; PRG-ROM page 0 to bank 0
	stx MMC5_PRG_MODE3_P1	; PRG-ROM page 1 to bank 0
	stx MMC5_PRG_MODE3_P2	; PRG-ROM page 2 to bank 0

	ldx #$0f		;
	stx MMC5_PRG_ROM_PM	; PRG-ROM permanent page to bank 15

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
reset_stub "STUB04"
reset_stub "STUB05"
reset_stub "STUB06"
reset_stub "STUB07"
reset_stub "STUB08"
reset_stub "STUB09"
reset_stub "STUB10"
reset_stub "STUB11"
reset_stub "STUB12"
reset_stub "STUB13"
reset_stub "STUB14"
reset_stub "STUB15"
