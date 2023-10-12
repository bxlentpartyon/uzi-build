void psleep(void *event);
void sendsig(ptptr proc, int16 sig);
void wakeup(char *event);
void ssig(register ptptr proc, int16 sig);
void chksigs(void);
void swapin(ptptr pp);
ptptr getproc(void);
int dofork(void);
int clk_int(void);

/* only referenced inside process.c */
void newproc(ptptr p);
void swrite(void);
