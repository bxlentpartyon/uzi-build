	.import		callptr4
	.export		_trampoline

.segment "CODE"

.proc _trampoline
	jsr callptr4
	rts
.endproc
