UZI_KERNEL_DIR := $(UZIBUILD_ROOT)/uzi-nes-port
UZI_INTERNAL_CFLAGS := -nostdinc -isystem $(UZI_KERNEL_DIR)/include -DUZI -DUZI_UTIL_BUILD

FSUTIL_TARGETS := fsutil_filesys.o fsutil_chmod.o fsutil_get.o fsutil_kdata.o fsutil_ls.o fsutil_machdep.o fsutil_mkdir.o fsutil_process.o fsutil_time.o fsutil_type.o fsutil_wd.o fsutil_rm.o
KERNEL_TARGETS := linux_devio.o linux_filesys.o linux_scall1.o

fsutil_all.o: fsutil.o fsutil_lib.o $(FSUTIL_TARGETS) $(KERNEL_TARGETS)
	ar rv $@ $^

$(FSUTIL_TARGETS): %.o : %.c
	$(CC) $(UZI_INTERNAL_CFLAGS) -c -g -o $@ $<

linux_devio.o: $(UZI_KERNEL_DIR)/drivers/devio.c
linux_filesys.o: $(UZI_KERNEL_DIR)/fs/filesys.c
linux_scall1.o: $(UZI_KERNEL_DIR)/kernel/scall1.c

$(KERNEL_TARGETS):
	$(CC) $(UZI_INTERNAL_CFLAGS) -c -g -o $@ $<

%.o: %.c
	$(CC) -c -g -o $@ $<

clean:
	rm -f *.o
