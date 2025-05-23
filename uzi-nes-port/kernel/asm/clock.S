	.export		_apu_status_byte
	.export		_start_clock, _stop_clock

	.include	"uzi_nes.inc"

.segment "USR_ZEROPAGE": zeropage
_apu_status_byte:
	.byte $00

.segment "CODE"

; start/stop_clock are void functions, so it's OK if they clobber A
.proc _start_clock
	lda #%00000000
	sta APU_FRAME_COUNTER	; enable clock interrupt
	rts
.endproc

.proc _stop_clock
	lda #%01000000
	sta APU_FRAME_COUNTER	; disable clock interrupt
	rts
.endproc
