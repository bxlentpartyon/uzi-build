#ifndef __DEVWD_H__
#define __DEVWD_H__

/* static to devwd.c */
int setup(unsigned minor, int rawflag);
void chkstat(int stat, int rdflag);

#endif /* __DEVWD_H__ */
