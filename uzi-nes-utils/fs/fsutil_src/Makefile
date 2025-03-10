UZI_KERNEL_DIR := $(UZIBUILD_ROOT)/uzi-nes-port
UZI_INTERNAL_CFLAGS := -nostdinc -isystem $(UZI_KERNEL_DIR)/include -DUZI

fsutil_all.o: fsutil.o fsutil_filesys.o fsutil_kdata.o fsutil_ls.o fsutil_machdep.o fsutil_mkdir.o fsutil_process.o fsutil_time.o fsutil_wd.o linux_devio.o linux_filesys.o linux_scall1.o
	ar rv $@ $^

fsutil_filesys.o: fsutil_filesys.c
	$(CC) $(UZI_INTERNAL_CFLAGS) -c -g -o $@ $<

fsutil_kdata.o: fsutil_kdata.c
	$(CC) $(UZI_INTERNAL_CFLAGS) -c -g -o $@ $<

fsutil_ls.o: fsutil_ls.c
	$(CC) $(UZI_INTERNAL_CFLAGS) -c -g -o $@ $<

fsutil_machdep.o: fsutil_machdep.c
	$(CC) $(UZI_INTERNAL_CFLAGS) -c -g -o $@ $<

fsutil_mkdir.o: fsutil_mkdir.c
	$(CC) $(UZI_INTERNAL_CFLAGS) -c -g -o $@ $<

fsutil_process.o: fsutil_process.c
	$(CC) $(UZI_INTERNAL_CFLAGS) -c -g -o $@ $<

fsutil_time.o: fsutil_time.c
	$(CC) $(UZI_INTERNAL_CFLAGS) -c -g -o $@ $<

fsutil_wd.o: fsutil_wd.c
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
