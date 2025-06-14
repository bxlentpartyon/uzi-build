	.autoimport	on

.segment "HEAD"
	jmp start

.segment "DATA"
tty_path:
	.asciiz "/dev/tty"
ret:
	.word $0000
status:
	.word $0000

.segment "CODE"
start:
; open("/dev/tty", O_RDWR)
	; argn3, argn2
	lda #$00
	ldx #$00
	jsr pushax
	jsr pushax

	; argn1
	lda #<tty_path
	ldx #>tty_path
	jsr pushax

	; argn
	lda #$02
	ldx #$00
	jsr pushax

	; uret
	lda #<ret
	ldx #>ret
	jsr pushax

	; callno 1
	lda #$01
	ldx #$00

	; jump to dispatch vector
	jsr $0300

	sta status
	stx status+1

forever:
	jmp forever
