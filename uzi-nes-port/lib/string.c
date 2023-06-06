/* Super slow bzero */
void bzero(char *p, unsigned int s)
{
	unsigned int i;

	for (i = 0; i < s; i++)
		p[i] = '\0';
}
