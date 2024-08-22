UTIL_OBJS := mkfs.o
UTIL_TARGETS := mkfs

ifeq ($(origin UZIBUILD_ROOT), undefined)
$(error uzibuild environment is not set up)
endif

all: mkfs fsck fsutil

linux_fs.o: linux_fs.c linux_fs.h
	gcc -I$(UZIBUILD_ROOT)/uzi-nes-port/include -c -o $@ $<

mkfs: mkfs.c linux_fs.o linux_fs.h
	gcc -I$(UZIBUILD_ROOT)/uzi-nes-port/include -o $@ $< linux_fs.o

fsck: fsck.c linux_fs.o linux_fs.h
	gcc -I$(UZIBUILD_ROOT)/uzi-nes-port/include -o $@ $< linux_fs.o

fsutil: fsutil.c linux_fs.o linux_fs.h
	gcc -I$(UZIBUILD_ROOT)/uzi-nes-port/include -o $@ $< linux_fs.o

clean:
	rm mkfs fsck fsutil linux_fs.o
