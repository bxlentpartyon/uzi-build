void sendsig(ptptr proc, int16 sig);
void ssig(register ptptr proc, int16 sig);
void swapin(ptptr pp);
int dofork(void);
int clk_int(void);

/* only referenced inside process.c */
void newproc(ptptr p);
void swrite(void);
