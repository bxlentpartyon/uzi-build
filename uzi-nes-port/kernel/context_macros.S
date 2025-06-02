.macro _zpsave	dest
	ldy #uzi_zpsavespace

	; Store the jmpvec first
	lda jmpvec+1
	sta dest, y
	dey
	lda jmpvec+2
	sta dest, y
	dey

	; Now loop over the rest
zpsave_loop:
	lda sp, y
	sta dest, y
	dey
	bpl zpsave_loop
.endmacro

.macro _zprestore	dest
	ldy #uzi_zpsavespace

	; Restore the jmpvec first
	lda dest, y
	sta jmpvec+2
	dey
	lda dest, y
	sta jmpvec+1
	dey

	; Now loop over the rest
zprestore_loop:
	lda dest, y
	sta sp, y
	dey
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
