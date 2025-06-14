	.importzp	sp
	.import 	_start_kernel, _stop_clock
	.import		ppu_ctrl_val
	.import		cur_code_bank, cur_stack_page
        .import         _unix
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

.segment "DISPATCH_TRAMP"
dispatch_trampoline:  .res 16, $00

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
	sta $a000, x
	lda $8100, x
	sta $a100, x
	lda $8200, x
	sta $a200, x
	lda $8300, x
	sta $a300, x
	lda $8400, x
	sta $a400, x
	lda $8500, x
	sta $a500, x
	lda $8600, x
	sta $a600, x
	lda $8700, x
	sta $a700, x
	lda $8800, x
	sta $a800, x
	lda $8900, x
	sta $a900, x
	lda $8a00, x
	sta $aa00, x
	lda $8b00, x
	sta $ab00, x
	lda $8c00, x
	sta $ac00, x
	lda $8d00, x
	sta $ad00, x
	lda $8e00, x
	sta $ae00, x
	lda $8f00, x
	sta $af00, x
	lda $9000, x
	sta $b000, x
	lda $9100, x
	sta $b100, x
	lda $9200, x
	sta $b200, x
	lda $9300, x
	sta $b300, x
	lda $9400, x
	sta $b400, x
	lda $9500, x
	sta $b500, x
	lda $9600, x
	sta $b600, x
	lda $9700, x
	sta $b700, x
	lda $9800, x
	sta $b800, x
	lda $9900, x
	sta $b900, x
	lda $9a00, x
	sta $ba00, x
	lda $9b00, x
	sta $bb00, x
	lda $9c00, x
	sta $bc00, x
	lda $9d00, x
	sta $bd00, x
	lda $9e00, x
	sta $be00, x
	lda $9f00, x
	sta $bf00, x
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
	stx cur_stack_page
	stx MMC5_PRG_MODE3_P0	; RAM Page 0 to bank 7

	; clear the kernel stack data
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

init_dispatch:
	; Write the following instructions to the trampoline:
	;   jsr _unix
	;   rts
        lda #$20
        sta dispatch_trampoline
        lda #<_unix
        sta dispatch_trampoline+1
        lda #>_unix
        sta dispatch_trampoline+2
	lda #$60
	sta dispatch_trampoline+3

	cli

	jmp _start_kernel
.endproc
