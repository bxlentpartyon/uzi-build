	.importzp	sp
	.import		jmpvec
	.importzp	_apu_status_byte
	.import		_handle_irq, _handle_vblank
	.export		_ei, _di, nmi, irq

	.include	"uzi_nes.inc"
	.include	"zeropage.inc"

.segment "DATA"
zpsave: .res zpsavespace
zpsave_size: .byte $14

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
	ldx #$14
zpsave_loop:
	lda $15, x
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
	ldx #$14
zprestore_loop:
	lda zpsave, x
	sta $15, x
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

.proc nmi
	save_regs
	activate_irq_stack
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

