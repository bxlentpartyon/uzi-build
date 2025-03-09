void di(void);
void ei(void);
void idump(void);
void tempstack(void);
void doexec(void *new_stack);
void panic(char *s);
void stkreset(void);
void initvec(void);
void calltrap(void);

/* time functions */
void rdtime(time_t *tloc);

/* Not actually implemented yet */
unsigned int in(unsigned int *addr);
void out(char c, unsigned int *addr);
