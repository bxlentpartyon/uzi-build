UZI_KERNEL_DIR := $(UZIBUILD_ROOT)/uzi-nes-port
UZI_INTERNAL_CFLAGS := -nostdinc -isystem $(UZI_KERNEL_DIR)/include -DUZI

FSUTIL_TARGETS := fsutil_filesys.o fsutil_kdata.o fsutil_ls.o fsutil_machdep.o fsutil_mkdir.o fsutil_process.o fsutil_time.o fsutil_wd.o

fsutil_all.o: fsutil.o $(FSUTIL_TARGETS) linux_devio.o linux_filesys.o linux_scall1.o
	ar rv $@ $^

$(FSUTIL_TARGETS): %.o : %.c
	$(CC) $(UZI_INTERNAL_CFLAGS) -c -g -o $@ $<

linux_devio.o: $(UZI_KERNEL_DIR)/drivers/devio.c
	$(CC) $(UZI_INTERNAL_CFLAGS) -c -g -o $@ $<

linux_filesys.o: $(UZI_KERNEL_DIR)/fs/filesys.c
	$(CC) $(UZI_INTERNAL_CFLAGS) -c -g -o $@ $<

linux_scall1.o: $(UZI_KERNEL_DIR)/kernel/scall1.c
	$(CC) $(UZI_INTERNAL_CFLAGS) -c -g -o $@ $<

%.o: %.c
	$(CC) -c -g -o $@ $<

clean:
	rm -f *.o
