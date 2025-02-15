	.importzp	sp
	.import 	_start_kernel, _stop_clock
	.import		nmi, irq
	.import		ppu_ctrl_val
	.import		cur_code_bank
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

.proc dummy
	lda cur_code_bank
.endproc

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


	; copy init data from rom to ram
	ldx #$00
copy_to_ram:
	lda $c000, x
	sta $6000, x
	lda $c100, x
	sta $6100, x
	lda $c200, x
	sta $6200, x
	lda $c300, x
	sta $6300, x
	lda $c400, x
	sta $6400, x
	lda $c500, x
	sta $6500, x
	lda $c600, x
	sta $6600, x
	lda $c700, x
	sta $6700, x
	lda $c800, x
	sta $6800, x
	lda $c900, x
	sta $6900, x
	lda $ca00, x
	sta $6a00, x
	lda $cb00, x
	sta $6b00, x
	lda $cc00, x
	sta $6c00, x
	lda $cd00, x
	sta $6d00, x
	lda $ce00, x
	sta $6e00, x
	lda $cf00, x
	sta $6f00, x
	lda $d000, x
	sta $7000, x
	lda $d100, x
	sta $7100, x
	lda $d200, x
	sta $7200, x
	lda $d300, x
	sta $7300, x
	lda $d400, x
	sta $7400, x
	lda $d500, x
	sta $7500, x
	lda $d600, x
	sta $7600, x
	lda $d700, x
	sta $7700, x
	lda $d800, x
	sta $7800, x
	lda $d900, x
	sta $7900, x
	lda $da00, x
	sta $7a00, x
	lda $db00, x
	sta $7b00, x
	lda $dc00, x
	sta $7c00, x
	lda $dd00, x
	sta $7d00, x
	lda $de00, x
	sta $7e00, x
	lda $df00, x
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

	ldx #$07		;
	stx MMC5_PRG_MODE3_P0	; PRG-RAM page 0 to bank 10

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

	ldx #$07		;
	stx MMC5_PRG_ROM_PM	; PRG-ROM permanent page to bank 7

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
