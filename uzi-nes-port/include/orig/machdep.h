void di(void);
void ei(void);
void idump(void);
void warning(char *s);
int valadr(char *base, uint16 size);
void tempstack(void);
void doexec(void *new_stack);
void panic(char *s);
void stkreset(void);
void initvec(void);
void calltrap(void);

/* time functions */
void rdtime(time_t *tloc);
void rdtod(void);
void addtick(time_t *t1, time_t *t2);
void incrtick(time_t *t);

/* Not actually implemented yet */
unsigned int in(unsigned int *addr);
void out(char c, unsigned int *addr);

/* only referenced inside machdep.h */
uint16 tread(uint16 port);
void kputchar(int c);
