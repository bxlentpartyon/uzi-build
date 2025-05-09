	.importzp	sp
	.import		_di, _ei, _disable_vblank_nmi, _enable_vblank_nmi
	.import		cur_stack_page
	.import		pushax, popa
	.export		_udata, _break_addr, _break_state
	.export		_user_bank_setup, _tempstack, _doexec, _debug_break
	.export		_fork_prep_begin, _fork_prep_end, _swapout_prep, _swapin_finish

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

	; swap in second process memory bank (only one process for now)
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

; The swapout prep process is the same as for fork, but we use 1 for the
; return value instead of the new process pid.
;
; These could maybe be smashed together into a layered .proc to avoid the jmp,
; but I'm not sure if that will work as I expect, so it's implemented like
; this for now.
.proc _swapout_prep
	ldx #$00
	lda #$01
	jmp _fork_prep_begin
.endproc

; Stores new process pid (currently in A/X) at the top of the stack.  This
; will end up being appearing as the return value from fork() when the parent
; process is swapped back in.
;
; Returns the current stack pointer in A/X
.proc _fork_prep_begin
	jsr pushax
	ldy sp+1
	tya
	tax
	ldy sp
	tya
	rts
.endproc

.proc _fork_prep_end
	jsr popa
	jsr popa
.endproc

; Places the saved off stack pointer at sp, and loads the stored off return
; value from the new stack into A/X.  The rts here will bring us back to
; swapin, and the rts in swapin will drop us back to wherever the process was.
.proc _swapin_finish
	stx sp+1
	sta sp
	jsr popa
	tax
	jsr popa
	rts
.endproc
