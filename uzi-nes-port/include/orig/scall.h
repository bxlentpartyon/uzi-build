#ifndef __SCALL_H__
#define __SCALL_H__

int _execve(void);

/* stuff below is static to scall1.c and should be pulled out of the header */
void updoff(void);
void stcpy(inoptr ino, char *buf);

/* stuff below is static to scall2.c and should be pulled out of the header */
int wargs(char **argv, int blk);
void exec2(void);

#endif /* __SCALL_H__ */
