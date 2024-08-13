UTIL_OBJS := mkfs.o
UTIL_TARGETS := mkfs

ifeq ($(origin UZIBUILD_ROOT), undefined)
$(error uzibuild environment is not set up)
endif

all: mkfs fsck

fs.o: fs.c fs.h
	gcc -I$(UZIBUILD_ROOT)/uzi-nes-port/include -c -o $@ $<

mkfs: mkfs.c fs.o fs.h
	gcc -I$(UZIBUILD_ROOT)/uzi-nes-port/include -o $@ $< fs.o

fsck: fsck.c fs.o fs.h
	gcc -I$(UZIBUILD_ROOT)/uzi-nes-port/include -o $@ $< fs.o
