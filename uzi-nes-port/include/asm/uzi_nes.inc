.ifndef __UZI_NES_INC__
.define __UZI_NES_INC__

; PPU registers
PPU_CTRL	= $2000
PPU_MASK	= $2001
PPU_STATUS	= $2002
PPU_SCROLL	= $2005
PPU_ADDR	= $2006
PPU_DATA	= $2007

APU_STATUS		= $4015
APU_FRAME_COUNTER	= $4017

DMC_CTRL	= $4010

; Must be kept in sync with PROGBASE definition in include/unix.h
PROGBASE	= $6000

; Offsets into udata structure
OSYS		= $02

; Kernel memory information
KERNEL_STACK_PAGE	= $07
KERNEL_RAM_PAGE		= $08

KERNEL_INITDATA_ROM	= $80
KERNEL_FONTDATA_ROM	= $81
KERNEL_INITCODE_ROM	= $8e
KERNEL_COMMCODE_ROM	= $0f

.endif
