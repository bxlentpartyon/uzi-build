extern void print_hello(void);
extern void ppu_reset(void);

void start_kernel(void)
{
	while (1);
}

void handle_vblank(void)
{
	print_hello();
	ppu_reset();
}
