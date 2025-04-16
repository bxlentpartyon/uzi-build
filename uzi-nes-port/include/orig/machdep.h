void idump(void);
void doexec(void *new_stack);
void calltrap(void);

/* time functions */
void rdtime(time_t *tloc);

/* Not actually implemented yet */
unsigned int in(unsigned int *addr);
void out(char c, unsigned int *addr);
