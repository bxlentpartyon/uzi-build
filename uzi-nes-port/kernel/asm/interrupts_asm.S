	.importzp	sp
	.import		jmpvec
	.importzp	_apu_status_byte
	.import		_handle_irq, handle_vblank
	.import		ppu_ctrl_val
	.import		stack_setup
	.export		_ei, _di, nmi, irq, _in_interrupt
	.export		_enable_vblank_nmi, _disable_vblank_nmi
	.export		cur_stack_page

	.include	"mmc5.inc"
	.include	"uzi_nes.inc"
	.include	"zeropage.inc"

	.linecont

.segment "DATA"
zpsave:		.res zpsavespace
_in_interrupt:	.byte 0
irq_stack_start: .addr $8f00
cur_stack_page:	.byte $00

.segment "CODE"

.macro enable_vblank_nmi
	bit PPU_STATUS  ; read PPU_STATUS to clear vblank bit
	lda ppu_ctrl_val
	ora #%10000000
	sta ppu_ctrl_val
	sta PPU_CTRL	; enable vblank NMI
.endmacro

.macro disable_vblank_nmi
	lda ppu_ctrl_val
	and #%01111111
	sta ppu_ctrl_val
	sta PPU_CTRL	; disable vblank NMI
.endmacro

.proc _enable_vblank_nmi
	enable_vblank_nmi
	rts
.endproc

.proc _disable_vblank_nmi
	disable_vblank_nmi
	rts
.endproc

; enable interrupts
.proc _ei
	lda _in_interrupt
	bne ei_out
	cli
ei_out:
	rts
.endproc

; disable interrupts
.proc _di
	sei
	rts
.endproc

.macro activate_irq_stack
; Skip stack sanity check entirely for now. It makes a lot less sense with
; temp stacks and user stacks in play.  At very least, it needs to be a lot
; more clever.
.if 0
	lda stack_setup
	beq skip_stack_check
; make sure current stack pointer is above 0x6f00
	lda sp+1
	cmp irq_stack_start+1
; if the carry is clear then sp+1 <= 0x6f, which is too low in memory
	bcc bad_stack
.endif

skip_stack_check:
	; zpsave borrowed from libsrc/common/interrupt.s in cc65
	ldx #.sizeof(::zpsave)-1
zpsave_loop:
	lda sp, x
	sta zpsave, x
	dex
	bpl zpsave_loop

	lda jmpvec+1
	pha
	lda jmpvec+2
	pha

	lda irq_stack_start
	ldx irq_stack_start+1
	sta sp
	stx sp+1
	jmp out

bad_stack:
	jmp bad_stack

out:
	nop
.endmacro

.macro deacivate_irq_stack
	ldx #.sizeof(::zpsave)-1
zprestore_loop:
	lda zpsave, x
	sta sp, x
	dex
	bpl zprestore_loop

	pla
	sta jmpvec+2
	pla
	sta jmpvec+1
.endmacro

.macro save_regs
	pha	; save registers
	tya
	pha
	txa
	pha
.endmacro

.macro restore_regs
	pla	; restore registers
	tax
	pla
	tay
	pla
.endmacro

.macro begin_interrupt
; If I've done everything right, we shouldn't be able to get overlapped interrupts
; any more.  However, just to be safe, I've added this code to drop us into an infinite
; loop if that ever happens.
	lda _in_interrupt
	beq storeoff
forever:
	jmp forever

storeoff:
	pha
	lda #1
	sta _in_interrupt
.endmacro

.macro end_interrupt
	pla
	sta _in_interrupt
.endmacro

; The NMI routine, or at least the portion of it that manipulates the PPU,
; needs to be shorter than VBlank, which is ~2273 CPU cycles.  Otherwise, we'll
; see totally unpredictable behavior.
;
; https://forums.nesdev.org/viewtopic.php?t=7878
.proc nmi
; Note that we don't need to use an IRQ stack here, as the NMI handler is written
; entirely in assembly, so it won't touch the C stack.
	save_regs
	jsr handle_vblank
	restore_regs

	rti
.endproc

.proc irq
	save_regs
	begin_interrupt

	lda APU_STATUS
	sta _apu_status_byte

	ldx #KERNEL_STACK_PAGE
	stx MMC5_PRG_MODE3_P0

	activate_irq_stack
	jsr _handle_irq
	deacivate_irq_stack

	ldx cur_stack_page
	stx MMC5_PRG_MODE3_P0

	end_interrupt
	restore_regs

	rti
.endproc
