#include <ppu.h>
#include <lib/string.h>

/* Super slow bzero */
void bzero(char *p, unsigned int s)
{
	unsigned int i;

	for (i = 0; i < s; i++)
		p[i] = '\0';
}

/* Super slow bcopy */
void bcopy(char *src, char *dest, unsigned int n)
{
	int i;
	char tmp;

	/* Not sure if the tmp swap is necessary here */
	for (i = 0; i < n; i++) {
		tmp = src[i];
		dest[i] = tmp;
	}
}

int strlen(char *s)
{
	char *curpos = s;
	int curlen = 0;

	if (!curpos)
		return 0;

	while (*curpos != '\0') {
		curlen++;
		curpos++;
	}

	return curlen;
}

void reverse(char *s)
{
	int len = strlen(s);
	int i, j = len - 1;
	char temp;

	for (i = 0; i < len / 2; i++) {
		temp = s[i];
		s[i] = s[j];
		s[j] = temp;
		j--;
	}
}

void itob(int n, char *s, int b)
{
	char *cur_pos = s;
	int cur_digit;
	int base = (b < 0) ? -b : b;
	unsigned int cur_n = (unsigned int) (n < 0 && base == 10) ? -n : n;

	do {
		cur_digit = cur_n % base;
		if (cur_digit > 9)
			*(cur_pos++) = (cur_digit - 10) + 'a';
		else
			*(cur_pos++) = cur_digit + '0';
		cur_n = cur_n / base;
	} while (cur_n > 0);

	if (b < 0 && n < 0)
		*(cur_pos++) = '-';

	*(cur_pos)++ = '\0';

	reverse(s);
}
