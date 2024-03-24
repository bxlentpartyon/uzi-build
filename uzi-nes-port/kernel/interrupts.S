	.importzp	sp
	.import		jmpvec
	.importzp	_apu_status_byte
	.import		_handle_irq, _handle_vblank, _dump_keyboard
	.export		_ei, _di, nmi, irq, _kb_rows, _kb_map, _kb_checked

	.include	"uzi_nes.inc"
	.include	"zeropage.inc"

	.linecont

.segment "DATA"
zpsave:		.res zpsavespace
_kb_rows:	.res	 9
kb_tmp:		.byte	$00
_kb_map: .byte	$00, $01, '[', ']', $02, $03, '\', $04,	\
			$05, '@', ':', ';', '_', '/', '-', '^',		\
			$06, 'O', 'L', 'K', '.', ',', 'P', '0',		\
			$07, 'I', 'U', 'J', 'M', 'N', '9', '8',		\
			$08, 'Y', 'G', 'H', 'B', 'V', '7', '6',		\
			$09, 'T', 'R', 'D', 'F', 'C', '5', '4',		\
			$0a, 'W', 'S', 'A', 'X', 'Z', 'E', '3',		\
			$0b, $0c, 'Q', $0d, $0e, $0f, '1', '2',		\
			$10, $11, $12, $13, $14, $15, $16, $17
_kb_checked:	.byte $00

.segment "CODE"

.macro enable_vblank_nmi
	bit PPU_STATUS  ; read PPU_STATUS to clear vblank bit
	lda #%10000000
	sta PPU_CTRL	; enable vblank NMI
.endmacro

.macro disable_vblank_nmi
	lda #$00
	sta PPU_CTRL	; disable vblank NMI
.endmacro

; enable interrupts
.proc _ei
	cli
	enable_vblank_nmi
	rts
.endproc

; disable interrupts
.proc _di
	sei
	disable_vblank_nmi
	rts
.endproc

.macro activate_irq_stack
	; zpsave borrowed from libsrc/common/interrupt.s in cc65
	ldx #.sizeof(::zpsave)-1
zpsave_loop:
	lda sp, x
	sta zpsave, x
	dex
	bpl zpsave_loop

	lda jmpvec+1
	pha
	lda jmpvec+2
	pha

	lda #$00
	ldx #$06
	sta sp
	stx sp+1
.endmacro

.macro deacivate_irq_stack
	ldx #.sizeof(::zpsave)-1
zprestore_loop:
	lda zpsave, x
	sta sp, x
	dex
	bpl zprestore_loop

	pla
	sta jmpvec+2
	pla
	sta jmpvec+1
.endmacro

.macro save_regs
	pha	; save registers
	tya
	pha
	txa
	pha
.endmacro

.macro restore_regs
	pla	; restore registers
	tax
	pla
	tay
	pla
.endmacro

.proc read_keyboard
	lda #$05
	sta $4016	; reset keyboard reader to row 0

	ldx #$00
kb_read_loop:
	lda #$04
	sta $4016	; begin next row

	lda $4017	; load column 0 bits xxxH HHHx
	and #$1e	; mask out unused bits to be super-safe
	lsr a		; shift right for xxxx HHHH
	sta kb_tmp

	lda #$06
	sta $4016	; switch to column 1

	lda $4017	; load colum 1 bits xxxH HHHx
	and #$1e	; mask out unused bits again
	asl
	asl
	asl		; shift left for HHHH xxxx
	ora kb_tmp	; OR with the lower bits to get the full column byte in A

	sta _kb_rows, x	; store the computed row byte

	inx
	txa
	cmp #$09	; loop through all 8 columns
	bne kb_read_loop

	lda #$00
	sta _kb_checked

	rts
.endproc

.proc nmi
	save_regs
	activate_irq_stack

	jsr read_keyboard
	jsr _handle_vblank

	deacivate_irq_stack
	restore_regs

	rti
.endproc

.proc irq
	pha	; broken up register to save to disable vblank NMI ASAP
	disable_vblank_nmi
	tya
	pha
	txa
	pha

	lda APU_STATUS
	sta _apu_status_byte

	activate_irq_stack
	jsr _handle_irq
	deacivate_irq_stack
	enable_vblank_nmi
	restore_regs

	rti
.endproc

