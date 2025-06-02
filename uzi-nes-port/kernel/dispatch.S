        .importzp       sp
	.import		jmpvec
	.import		___dispatch_call
	.import		zpsave, cur_stack_page
	.export		_dispatch_call

	.include	"mmc5.inc"
	.include	"uzi_nes.inc"

	.include	"context_macros.S"

.segment "DATA"
ret_tmp:	.res 2

.segment "USR_ZEROPAGE": zeropage
zpsave_tmp:	.res 2

.segment "DISPATCH_CODE"
.proc _dispatch_call
	sta zpsave_tmp
	stx zpsave_tmp+1

	_zpsave (zpsave_tmp)

	ldy #KERNEL_STACK_PAGE
	sty MMC5_PRG_MODE3_P0

	jsr ___dispatch_call
	sta ret_tmp
	stx ret_tmp+1

	ldy cur_stack_page
	sty MMC5_PRG_MODE3_P0

	_zprestore (zpsave_tmp)

	lda ret_tmp
	ldx ret_tmp+1

	rts
.endproc
