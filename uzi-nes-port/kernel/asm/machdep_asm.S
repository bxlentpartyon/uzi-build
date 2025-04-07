	.importzp	sp
	.export		_udata
	.export		_tempstack, _doexec

	.include	"uzi_nes.inc"

.segment "UDATA"
_udata:
	.align $100
	.res $100, $00

.segment "DATA"
jump_temp:
	.addr $0000

.segment "TMP_STACK"
.define templen $100
temp_stack:
	.res templen, $00

.segment "SCALL2_CODE"
.proc _tempstack
	pla
	sta jump_temp+1
	pla
	sta jump_temp

	lda <temp_stack
	sta sp
	lda >temp_stack
	sta sp+1

	jmp (jump_temp)
.endproc

.proc _doexec
	; set the new stack pointer
	sta sp
	stx sp+1

	; signal that we're leaving the kernel
	lda #0
	sta _udata + OSYS

	; jump into the new program
	jmp PROGBASE
.endproc
