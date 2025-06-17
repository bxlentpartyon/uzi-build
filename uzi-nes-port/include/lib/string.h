#ifndef __STRING_H__
#define __STRING_H__

void __fastcall__ bzero(char *p, unsigned int s);
void bcopy(char *src, char *dest, unsigned int n);
void itob(int n, char *s, int b);
int strlen(char *s);

#endif				/* __STRING_H__ */
