	.importzp	sp
	.export		_udata
	.export		_tempstack

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
