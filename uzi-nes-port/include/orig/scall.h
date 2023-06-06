#ifndef __SCALL_H__
#define __SCALL_H__

void readi(register inoptr ino);
void writei(register inoptr ino);
int doclose(int16 uindex);
int _execve(void);
void _sync(void);

/* stuff below is static to scall1.c and should be pulled out of the header */
void updoff(void);
int min(int a, int b);
int psize(inoptr ino);
void addoff(off_t *ofptr, int amount);
void stcpy(inoptr ino, char *buf);

/* stuff below is static to scall2.c and should be pulled out of the header */
int wargs(char **argv, int blk);
void exec2(void);
void doexit(int16 val, int16 val2);

#endif /* __SCALL_H__ */
