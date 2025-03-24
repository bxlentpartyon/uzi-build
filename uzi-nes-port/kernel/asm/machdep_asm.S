	.importzp	sp
	.export		_tempstack

.segment "TMP_STACK"
.define templen $100
jump_temp:
	.addr $0000
temp_stack:
	.res templen, $00

.proc _tempstack
	pla
	sta jump_temp+1
	pla
	sta jump_temp

	lda <temp_stack
	sta sp
	lda >temp_stack
	sta sp+1

	jmp (jmp_temp)	
.endproc
