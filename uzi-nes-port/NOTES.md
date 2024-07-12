Current size of p_tab: 0x15 (21) bytes

# Handling multiple processes

## The problem

The NES doesn't have any kind of respectable external storage available to it.
The FDS (Famicom Disk System) provides 64K per disk-side of storage, but it
also needs a bunch of special hardware and a BIOS ROM to make it work, so it
can't be made to work with MMC5.  Even if it could, the 128K of storage provided
by a single disk isn't any better than the 128K of WRAM potentially provided by
the MMC5.

## Current WIP solution

### RAM

Use the 16 8K pages of WRAM as the entire working address space for each
process.  This leaves us with a maximum of 15 potential processes, since we need
one page for the kernel.  Realistically, we want to take advantage of the
battery-backing on the lower half of the WRAM for some R/W filesystem space, so
I'm currently thinking that I'll divide this in half:  The first 8 pages will be
an R/W filesystem, and the second 8 pages will be the kernel and 7 userspace
processes.

This solution is still just theoretical.  For now, I'm just not going to
implement swapping and get a single userspace process working.

### ROM

#### Layout

Code for each userspace process will live in one PRG ROM bank.  The kernel will
page in the appropriate process, and its RAM page, before jumping into the
userspace process.

#### What does this mean for the filesystem?

For now we need to have a read-only, program-only filesystem.  The FS metadata
will be a super-simple table, mapping a 16-character program name to a 1-byte
page number.  For now we'll just bake this into the kernel to get something
working.

## A real solution?

There is apparently some level of support for battery-backed CHR-RAM in FCEUX,
and we can theoretically page 4M of data in CHR-RAM.  Only 2M is battery-backed
(non-volatile), but this is more non-volatile storage than we have available
elsewhere.

I'm going to use the CHR-RAM like a disk, where a regular UZI filesystem will
live in the battery-backed portion.  This will require a properly formatted
filesystem in the uzi.nes .sav file, but that's trivial to cook up.  If I use
PRG-RAM for the 8000-BFFF page and load programs from CHR-RAM as needed, this
will allow me to more-or-less port UZI as it originally existed, but our "disk"
will actually be NVRAM accessed through the PPU.

### Deeper Investigation

R/W with the PPU is relatively slow because it can only be done during VBlank.
Currently, I only have about 300 spare CPU cycles in VBlank to do PPU R/W, so,
for a worst-case scenario R/W speed analysis, if we use indirect y-indexed
loads, and absolute stores (since we always know the address of the PPUDATA
regsiter), like this:

lda (addr), y    ; 6 cycles
sta $2007        ; 4 cycles

https://www.nesdev.org/wiki/6502_cycle_times

We get 10 CPU-cycles per byte.  With 300 cycles to spare, that means we can
write about 30 bytes per VBlank.  With about 60 VBlanks per second, we can get
maybe 1800b/s R/W speed out of the PPU without too much effort.

This can be significantly improved if we throttle down the screen buffer dump
code when there's nothing to write to the screen.  Currently the screen buffer
dump takes about 1200 CPU cycles.

That gives us potentially 1500 CPU cycles, which puts us at more like 150 bytes
per VBlank, or about 9000b/s, which is significantly better.  If we have to have
an 8KB process code limit to begin with, that will at least allow a program load
in about a second.

# Stash area

This is a place for stuff I know I wanted to save, but don't remember why.

## IRQ stack clearing code

<snip>
	; clear irq stack
.if 0
	ldx #$00
clear_stack_loop:
	lda #$00
	sta $5c00, x
	sta $5d00, x
	sta $5e00, x
	sta $5f00, x
	inx
	bne clear_stack_loop
.endif
</snip>
