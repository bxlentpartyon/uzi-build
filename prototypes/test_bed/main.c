#include "asm.h"
#include "libc.h"

void main(void)
{
	ppu_setup();
	put_str("HELLO, WORLD!");
	ppu_on();
	while(1);
}
