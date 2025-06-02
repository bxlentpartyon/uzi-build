.macro _zpsave	dest
	; zpsave borrowed from libsrc/common/interrupt.s in cc65
	ldx #.sizeof(::zpsave)-1
zpsave_loop:
	lda sp, x
	sta dest, x
	dex
	bpl zpsave_loop

	lda jmpvec+1
	pha
	lda jmpvec+2
	pha
.endmacro

.macro _zprestore	dest
	ldx #.sizeof(::zpsave)-1
zprestore_loop:
	lda dest, x
	sta sp, x
	dex
	bpl zprestore_loop

	pla
	sta jmpvec+2
	pla
	sta jmpvec+1
.endmacro

.macro activate_irq_stack
; Skip stack sanity check entirely for now. It makes a lot less sense with
; temp stacks and user stacks in play.  At very least, it needs to be a lot
; more clever.
.if 0
	lda stack_setup
	beq skip_stack_check
; make sure current stack pointer is above 0x6f00
	lda sp+1
	cmp irq_stack_start+1
; if the carry is clear then sp+1 <= 0x6f, which is too low in memory
	bcc bad_stack
.endif

skip_stack_check:
	_zpsave zpsave

	lda irq_stack_start
	ldx irq_stack_start+1
	sta sp
	stx sp+1
	jmp out

bad_stack:
	jmp bad_stack

out:
	nop
.endmacro

.macro deacivate_irq_stack
	_zprestore zpsave
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
