UTIL_OBJS := mkfs.o
UTIL_TARGETS := mkfs

ifeq ($(origin UZIBUILD_ROOT), undefined)
$(error uzibuild environment is not set up)
endif

all: mkfs fsck fsutil test

CFLAGS += -I$(UZIBUILD_ROOT)/uzi-nes-port/include -I$(UZIBUILD_ROOT)/uzi-nes-utils/fs/include
export CFLAGS
HEADERS=include/linux_fs.h

linux_fs.o: linux_fs.c $(HEADERS)
	gcc $(CFLAGS) -c -o $@ $<

mkfs: mkfs.c linux_fs.o $(HEADERS)
	gcc $(CFLAGS) -o $@ $< linux_fs.o

fsck: fsck.c linux_fs.o $(HEADERS)
	gcc $(CFLAGS) -o $@ $< linux_fs.o

FSUTIL_OBJS := fsutil_src/fsutil_all.o linux_fs.o
fsutil: $(FSUTIL_OBJS) $(HEADERS)
	$(MAKE) -C fsutil_src
	gcc -g -o $@ $(FSUTIL_OBJS)

test: test.c
	gcc $(CFLAGS) -o $@ $<

clean:
	rm -f mkfs fsck fsutil/fsutil linux_fs.o
