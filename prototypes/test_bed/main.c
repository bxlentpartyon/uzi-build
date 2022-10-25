#include "asm.h"
#include "libc.h"

void main(void)
{
	put_str("HELLO, WORLD!");
	ppu_on();
	while(1);
}
