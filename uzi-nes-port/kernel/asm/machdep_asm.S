	.importzp	sp
	.export		_tempstack

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
