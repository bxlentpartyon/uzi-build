CFLAGS += -I$(UZIBUILD_ROOT)/uzi-nes-port/include -I$(UZIBUILD_ROOT)/uzi-nes-utils/fs/include
export CFLAGS

UZI_KERNEL_DIR := $(UZIBUILD_ROOT)/uzi-nes-port
UZI_INTERNAL_CFLAGS := -nostdinc -isystem $(UZI_KERNEL_DIR)/include -DUZI

fsutil_all.o: fsutil.o fsutil_std.o linux_ls.o linux_kdata.o linux_devio.o
	ar rv $@ $^

linux_ls.o: linux_ls.c
	$(CC) $(UZI_INTERNAL_CFLAGS) -c -g -o $@ $<

linux_devio.o: $(UZI_KERNEL_DIR)/drivers/devio.c
	$(CC) $(UZI_INTERNAL_CFLAGS) -c -g -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -g -o $@ $<

clean:
	rm -f fsutil_all.o fsutil.o ls.o linux_devio.o
