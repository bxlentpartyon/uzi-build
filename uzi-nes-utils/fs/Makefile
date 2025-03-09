UTIL_OBJS := mkfs.o
UTIL_TARGETS := mkfs

ifeq ($(origin UZIBUILD_ROOT), undefined)
$(error uzibuild environment is not set up)
endif

all: mkfs fsck fsutil test

CFLAGS += -I$(UZIBUILD_ROOT)/uzi-nes-port/include -I$(UZIBUILD_ROOT)/uzi-nes-utils/fs/include
HEADERS=include/linux_fs.h

linux_fs.o: linux_fs.c $(HEADERS)
	gcc $(CFLAGS) -c -o $@ $<

mkfs: mkfs.c linux_fs.o $(HEADERS)
	gcc $(CFLAGS) -o $@ $< linux_fs.o

fsck: fsck.c linux_fs.o $(HEADERS)
	gcc $(CFLAGS) -o $@ $< linux_fs.o

fsutil: fsutil/fsutil.c linux_fs.o $(HEADERS)
	gcc -g $(CFLAGS) -o $@ $< linux_fs.o

test: test.c
	gcc $(CFLAGS) -o $@ $<

clean:
	rm -f mkfs fsck fsutil/fsutil linux_fs.o
