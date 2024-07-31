UTIL_OBJS := mkfs.o
UTIL_TARGETS := mkfs

ifeq ($(origin UZIBUILD_ROOT), undefined)
$(error uzibuild environment is not set up)
endif

all: mkfs

mkfs: mkfs.c
	gcc -I$(UZIBUILD_ROOT)/uzi-nes-port/include -o $@ $<
