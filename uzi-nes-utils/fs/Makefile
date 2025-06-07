UTIL_OBJS := mkfs.o
UTIL_TARGETS := mkfs

ifeq ($(origin UZIBUILD_ROOT), undefined)
$(error uzibuild environment is not set up)
endif

all: mkfs fsck fsutil test

UTIL_CFLAGS += -I$(UZIBUILD_ROOT)/uzi-nes-utils/fs/include -I$(UZIBUILD_ROOT)/uzi-nes-port/include

HEADERS=include/linux_fs.h

linux_fs.o: linux_fs.c $(HEADERS)
	gcc $(UTIL_CFLAGS) -c -o $@ $<

mkfs: mkfs.c linux_fs.o $(HEADERS)
	gcc $(UTIL_CFLAGS) -o $@ $< linux_fs.o

fsck: fsck.c linux_fs.o $(HEADERS)
	gcc $(UTIL_CFLAGS) -o $@ $< linux_fs.o

fsutil_src/fsutil_all.o:
	$(MAKE) -C fsutil_src

FSUTIL_OBJS := fsutil_src/fsutil_all.o linux_fs.o
fsutil: $(FSUTIL_OBJS) $(HEADERS)
	gcc -g -o $@ $(FSUTIL_OBJS)

test: test.c
	gcc $(CFLAGS) $(UTIL_CFLAGS) -o $@ $<

clean:
	rm -f mkfs fsck fsutil linux_fs.o
	$(MAKE) -C fsutil_src clean

.PHONY: fsutil_src/fsutil_all.o
