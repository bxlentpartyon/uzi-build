	.import		_dump_keyboard
	.export		_read_keyboard, _kb_rows, _kb_map, _kb_checked

	.linecont

.define KB_ROW_FLIP_MASK	%11111111

.segment "DATA"
_kb_rows:	.res	 9
kb_tmp:		.byte	$00
kb_enabled:	.byte	$00
kb_key_pressed:	.byte	$00

;$4017	bit	1	2	3	4	1	2	3	4
;Row 0		F8	RETURN	[	]	KANA	RSHIFT	¥	STOP
;Row 1		F7	@	:	;	_	/	-	^
;Row 2		F6	O	L	K	.	,	P	0
;Row 3		F5	I	U	J	M	N	9	8
;Row 4		F4	Y	G	H	B	V	7	6
;Row 5		F3	T	R	D	F	C	5	4
;Row 6		F2	W	S	A	X	Z	E	3
;Row 7		F1	ESC	Q	CTR	LSHIFT	GRPH	1	2
;Row 8		CLRHOME	UP	RIGHT	LEFT	DOWN	SPACE	DEL	INS
_kb_map: .byte		$00, $0d, '[', ']', $02, $03, '\', $04,		\
			$05, '@', ':', ';', '_', '/', '-', '^',		\
			$06, 'O', 'L', 'K', '.', ',', 'P', '0',		\
			$07, 'I', 'U', 'J', 'M', 'N', '9', '8',		\
			$08, 'Y', 'G', 'H', 'B', 'V', '7', '6',		\
			$09, 'T', 'R', 'D', 'F', 'C', '5', '4',		\
			$0a, 'W', 'S', 'A', 'X', 'Z', 'E', '3',		\
			$0b, $0c, 'Q', $01, $0e, $0f, '1', '2',		\
			$10, $11, $12, $13, $14, $15, $16, $17
_kb_checked:	.byte $01

.proc check_kb_enabled
	; this subroutine assumes that the keyboard has been enabled
	; in software, and is set to row zero, i.e. $4016 == #$05
	;
	; we use the "Lode Runner method" to see if the kb is enabled:
	; https://www.nesdev.org/wiki/Family_BASIC_Keyboard#Detection
	ldx #$00
kb_skip_loop:
	lda #$06	; advance to column 1
	sta $4016
	lda #$04	; advance to next row

	inx
	txa
	cmp #$0a
	bne kb_skip_loop

	; at this point we should be at row 10, column 0
	lda #$1e	;
	and $4017	; $4017 should be set to 0x1e if the keyboard is connected
	cmp #$1e	;
	bne kb_enabled_out

	lda #$01
	sta kb_enabled

kb_enabled_out:
	rts
.endproc

.proc _read_keyboard
	lda #$05
	sta $4016	; reset keyboard reader to row 0

	lda #0			; this needs to be reset early so that we
	sta kb_key_pressed	; return something sane, even when we bail out

	lda kb_enabled		; if kb_enabled is set, jump to the loop
	bne kb_is_enabled	;

	jsr check_kb_enabled	; check if the kb has been enabled
	lda kb_enabled		; if not, bail out
	beq read_keyboard_out

kb_is_enabled:
	ldx #$00
kb_read_loop:
	lda #$04
	sta $4016	; begin next row

	lda $4017	; load column 0 bits xxxH HHHx
	and #$1e	; mask out unused bits to be super-safe
	lsr a		; shift right for xxxx HHHH
	sta kb_tmp

	lda #$06
	sta $4016	; switch to column 1

	lda $4017	; load colum 1 bits xxxH HHHx
	and #$1e	; mask out unused bits again
	asl
	asl
	asl		; shift left for HHHH xxxx
	ora kb_tmp		; OR with the lower bits to get the full column byte in A
	eor #KB_ROW_FLIP_MASK	; XOR the row bits with 0xFF to invert them (i.e. bit set
				; means key held)

	; check if anything was pressed
	beq store_computed_row
	ldy #01
	sty kb_key_pressed

store_computed_row:
	sta _kb_rows, x	; store the computed row byte

	inx
	txa
	cmp #$09	; loop through all 8 columns
	bne kb_read_loop

	lda #$00
	sta _kb_checked

read_keyboard_out:
	ldx #0			; high byte of return is always 0 here
	lda kb_key_pressed	; low byte of return is key_pressed bit
	rts
.endproc

