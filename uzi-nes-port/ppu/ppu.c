#include <interrupts.h>
#include <machdep.h>
#include <ppu.h>

extern void ppu_load_font(void);
extern void ppu_init_databuf(void);
extern void enable_vblank_nmi();
#ifdef PPU_SCREENBUF
extern void ppu_init_screenbuf(void);
#endif

int in_panic = 0;

void ppu_lock(void)
{
	/*
	 * There's no scenario where this lock should already be held.  If we're
	 * in interrupt context, we should have already checked that it isn't held.
	 * If we're in process context, there's a locking error somewhere, because
	 * we don't have multiple processes yet.
	 *
	 * This lock will probably need to become a proper mutex later on, but this
	 * works until something more complicated is actually needed.
	 */
	if (in_interrupt && ppu_locked)
		panic("ppu interrupt deadlock");

	if (ppu_locked)
		panic("ppu deadlock");

	ppu_locked = 1;
}

void ppu_unlock(void)
{
	ppu_locked = 0;
}

void init_ppu(void)
{
	ppu_load_font();
	ppu_init_databuf();
	enable_vblank_nmi();
	ppu_unlock();
#ifdef PPU_SCREENBUF
	ppu_init_screenbuf();
#endif
}

void ppu_puts(char *s)
{
	int pos = 0;
	char cur_char = *s;

	while (cur_char != 0x00) {
		ppu_putc(cur_char);
		cur_char = s[++pos];
	}
}

#ifdef PPU_PANIC
/* I'm leaving this around, but I'm moving to a more robust panic function */
void panic(char *msg)
{
	/* prevent recursion from ppu_put* functions */
	if (!in_panic) {
		in_panic = 1;
#ifdef PPU_SCREENBUF
		cursor_pos = 0;
#endif
		ppu_puts("PANIC: ");
		ppu_puts(msg);
		while(1);
	}
}
#endif
