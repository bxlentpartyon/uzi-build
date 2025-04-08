	.importzp	sp
	.import 	_start_kernel, _stop_clock
	.import		ppu_ctrl_val
	.import		cur_code_bank
	.export		reset
	.export		stack_setup

	.include	"uzi_nes.inc"
	.include	"mmc5.inc"

.segment "HEADER"
	.byte $4e, $45, $53, $1a	; 0-3	NES<EOF>
	.byte 8				; 4	PRG-ROM Banks
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

.segment "INIT_CODE"

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
	lda $8000, x
	sta $c000, x
	lda $8100, x
	sta $c100, x
	lda $8200, x
	sta $c200, x
	lda $8300, x
	sta $c300, x
	lda $8400, x
	sta $c400, x
	lda $8500, x
	sta $c500, x
	lda $8600, x
	sta $c600, x
	lda $8700, x
	sta $c700, x
	lda $8800, x
	sta $c800, x
	lda $8900, x
	sta $c900, x
	lda $8a00, x
	sta $ca00, x
	lda $8b00, x
	sta $cb00, x
	lda $8c00, x
	sta $cc00, x
	lda $8d00, x
	sta $cd00, x
	lda $8e00, x
	sta $ce00, x
	lda $8f00, x
	sta $cf00, x
	lda $9000, x
	sta $d000, x
	lda $9100, x
	sta $d100, x
	lda $9200, x
	sta $d200, x
	lda $9300, x
	sta $d300, x
	lda $9400, x
	sta $d400, x
	lda $9500, x
	sta $d500, x
	lda $9600, x
	sta $d600, x
	lda $9700, x
	sta $d700, x
	lda $9800, x
	sta $d800, x
	lda $9900, x
	sta $d900, x
	lda $9a00, x
	sta $da00, x
	lda $9b00, x
	sta $db00, x
	lda $9c00, x
	sta $dc00, x
	lda $9d00, x
	sta $dd00, x
	lda $9e00, x
	sta $de00, x
	lda $9f00, x
	sta $df00, x
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

	ldx #KERNEL_STACK_PAGE	; Kernel stack space
	stx MMC5_PRG_RAM_PM	; PRG-RAM permanent page to bank 7

	ldx #$00
clrmem2:
	lda #$00
	sta $6000, x
	sta $6100, x
	sta $6200, x
	sta $6300, x
	sta $6400, x
	sta $6500, x
	sta $6600, x
	sta $6700, x
	sta $6800, x
	sta $6900, x
	sta $6a00, x
	sta $6b00, x
	sta $6c00, x
	sta $6d00, x
	sta $6e00, x
	sta $6f00, x
	sta $7000, x
	sta $7100, x
	sta $7200, x
	sta $7300, x
	sta $7400, x
	sta $7500, x
	sta $7600, x
	sta $7700, x
	sta $7800, x
	sta $7900, x
	sta $7a00, x
	sta $7b00, x
	sta $7c00, x
	sta $7d00, x
	sta $7e00, x
	sta $7f00, x
	inx
	beq init_stack
	jmp clrmem2

init_stack:
	lda #$00
	ldx #$7f
	sta sp
	stx sp+1

	lda #01
	sta stack_setup

	cli

	jmp _start_kernel
.endproc
