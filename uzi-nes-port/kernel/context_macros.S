.macro _zpsave	dest
	; zpsave borrowed from libsrc/common/interrupt.s in cc65
	ldx #.sizeof(::zpsave)-1

	; Store the jmpvec first
	lda jmpvec+1
	sta dest, x
	dex
	lda jmpvec+2
	sta dest, x
	dex

	; Now loop over the rest
zpsave_loop:
	lda sp, x
	sta dest, x
	dex
	bpl zpsave_loop
.endmacro

.macro _zprestore	dest
	ldx #.sizeof(::zpsave)-1

	; Restore the jmpvec first
	lda dest, x
	sta jmpvec+2
	dex
	lda dest, x
	sta jmpvec+1
	dex

	; Now loop over the rest
zprestore_loop:
	lda dest, x
	sta sp, x
	dex
	bpl zprestore_loop
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
