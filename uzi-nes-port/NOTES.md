# Current memory bank layout

I'm currently using PRG mode 3, i.e. 5 8K banks.

That current bank layout is:

	Bank	Addres Range	Internal page	Usage
	------------------------------------------------------------------
	Bank 0	$6000-7FFF	RAM 08		Kernel RAM
	Bank 1	$8000-9FFF	RAM 07		Kernel stacks
	Bank 2	$A000-BFFF	-		Currently unused
	Bank 3	$C000-DFFF	ROM XX		Kernel floating code page
	Bank 4	$E000-FFFF	ROM 15		Kernel permanent code page

## New layout to get kernel actually working

I've realized that there's a big problem with all of the math surrounding
userspace memory layout because I've put kernel RAM at the lowest memory bank.
Even though we've now got `_data` at the beginning of RAM, the math for setting
up/swapping processes doesn't work unless udata actually lives at the very high
end of available memory.

Specifically the rargs stuff used for `_execve` does all of its math of where to
place the argv/env based off of udata being the highest address, and it also
sets the "break" (the area of memory between the process code and the
argv/environment/stack) based on these locations.

While this isn't efficient use of memory on the NES, if we don't put `_udata` at
a high address, all this math won't work, and we'll be redesigning stuff as we
try to port it, which isn't ideal.  Instead, I will move kernel RAM to be just
below the permanent ROM page to facilitate this.  For now, this should work
fine, as we won't be getting anywhere close to the limit of process size to
begin with.

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

# How will syscalls work?

I've had to think about this a number of times, so I want to get it written down
before I forget.  In the UZI180 kernel that I've looked at, a syscall is
triggered by a 'rst 30h' instruction.  We don't have that instruction, so I
think the best thing to do is to have a set address in the kernel memory page
that we jump to in order to start kernel entry, i.e. set everything up to call
unix().

The syscall address will have to be at a known location in the kernel memory
page, so I'll need to explicitly place it with .org.  The actual address can be
wherever, because we can use an indirect jump through the fixed address pointer
to reach it, if that makes sense.

# Known bugs

## Linker alignment problems with multiple sections in one bank

I'm not sure if this is a bug or if I'm just not understanding how something
works, but I started seeing a weird problem when I tried to move the udata stuff
to the beginning of kernel memory.

The linker changes I made look like this:

       UDATA:          load = PRG_ROM00, run = PRG_RAM, type = rw, define = yes, align = $100;

And the code is:

	.segment "UDATA"
	_udata:
		.align $100
		.res $e0, $DB

In my mind, I thought it would align the following symbols to 0x100 as well,
effectively padding out this section to 0x100 bytes, but it doesn't do that.
Instead, it's slamming the next pieces of data right up against the end of the
udata stuff.

Here's a dump from the resulting ROM, showing this:

	00000000  4e 45 53 1a 08 00 52 08  00 00 aa ff 00 00 00 00  |NES...R.........|
	00000010  db db db db db db db db  db db db db db db db db  |................|
	*
	000000f0  00 00 a9 72 00 00 00 00  64 f2 a6 f0 aa f2 86 f3  |...r....d.......|
	00000100  a6 f0 00 10 64 f2 a6 f0  aa f2 86 f3 a6 f0 00 00  |....d...........|

You can see the repeated 0xdb's there, until 0xf0, and then "some other stuff"
starts showing up right after that.  The weird thing is, it seems like the
linker treats symbol addresses as if this _didn't_ happen, so, in the case
above, all of the symbol addresses after this will be off by 16 bytes, because
we started emitting DATA 16 bytes early.

I think this probably has something to do with how I've sloppily pieced together
the data section.  It's likely that I've just been getting lucky up until now,
and I just exposed the nastiness when I started shuffling stuff around.

One possible solution might be to create explicit memory banks for some things,
to isolate the alignment restrictions to smaller chunks of memory.  I don't
actually know if this would work though...

For now, everything seems to play nice if I just allocate 256 bytes for udata,
but I should probably figure out the weird behavior here at some point.

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

## Current p_tab size

I don't remember why I thought I needed to save this...

Current size of p_tab: 0x15 (21) bytes
