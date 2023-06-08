/* Super slow bzero */
void bzero(char *p, unsigned int s)
{
	unsigned int i;

	for (i = 0; i < s; i++)
		p[i] = '\0';
}

int strlen(char *s)
{
	char *curpos = s;
	int curlen;

	while (*curpos++ != '\0')
		curlen++;

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
	int cur_n = (n < 0) ? -n : n;
	int base = (b < 0) ? -b : b;

	do {
		cur_digit = cur_n % base;
		*(cur_pos++) = cur_digit + '0';
		cur_n = cur_n / base;
	} while (cur_n > 0);

	if (b < 0 && n < 0)
		*(cur_pos++) = '-';

	*(cur_pos)++ = '\0';

	reverse(s);
}
