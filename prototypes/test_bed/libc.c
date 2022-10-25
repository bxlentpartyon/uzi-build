#include <string.h>

#include "asm.h"
#include "libc.h"

void put_str(char *str)
{
	int i, len;

	if (!str)
		return;

	reset_cursor();

	len = strlen(str);
	for (i = 0; i < len; i++)
		put_chr(str[i] - 0x20);
}
