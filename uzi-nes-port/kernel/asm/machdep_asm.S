	.importzp	sp
	.import		_di, _ei, _disable_vblank_nmi, _enable_vblank_nmi
	.import		cur_stack_page
	.export		_udata, _break_addr, _break_state
	.export		_user_bank_setup, _tempstack, _doexec, _debug_break

	.include	"mmc5.inc"
	.include	"uzi_nes.inc"

.segment "UDATA"
_udata:
	.align $100
	.res $100, $00

.segment "DATA"
jump_temp:
	.addr $0000
_break_addr:
	.addr $0000
_break_state:
	.addr $0000

.segment "TMP_STACK"
.define templen $100
temp_stack:
	.res templen, $00

.segment "CODE"
.proc _user_bank_setup
	; TODO these should be macros
	jsr _di

	; swap in first process memory bank (only one process for now)
	ldx #$09
	stx MMC5_PRG_RAM_PM

	; TODO these should be macros
	jsr _ei

	rts
.endproc

.proc _tempstack
	pla
	sta jump_temp
	pla
	sta jump_temp+1

	lda #<temp_stack
	adc #$ff
	sta sp
	lda #>temp_stack
	sta sp+1

	; TODO these should be macros
	jsr _di

	; swap in first process memory bank (only one process for now)
	ldx #$0a
	stx cur_stack_page
	stx MMC5_PRG_MODE3_P0

	; TODO these should be macros
	jsr _ei

	; we need to increment the "return" address by 1 to get to a proper
	; instruction offset
	inc jump_temp
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

.proc _debug_break
	sta _break_state
	stx _break_state+1	
	
	pla
	sta _break_addr
	pla
	sta _break_addr+1

	brk
.endproc
