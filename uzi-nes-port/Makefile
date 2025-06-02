CA := ca65
CC := cc65
LD := ld65

CC_INCLUDE := -I$(PWD)/include
CA_INCLUDE := -I$(PWD)/include/asm

PLATFORM := uzi
MAKEFILE_BUILD := $(PWD)/scripts/build/Makefile.build
ASDEPS_SCRIPT := $(PWD)/scripts/build/asdeps.sh
LINKDEPS := link.d

FONT :=	font/newfont.chr

# Default rule
TARGET := uzi.nes
all: $(TARGET)

# Setup for Makefile.build
export CA CC LD
export CC_INCLUDE CA_INCLUDE
export PLATFORM MAKEFILE_BUILD ASDEPS_SCRIPT LINKDEPS

subdirs := drivers fs include kernel lib ppu
-include $(MAKEFILE_BUILD)

# Round up all the .o's to link together
-include $(LINKDEPS)

LINKER_SCRIPT := scripts/linker/uzi.cfg
$(TARGET): $(O_DEPS)
	$(LD) -v -m link.map -vm -C $(LINKER_SCRIPT) -o $@ $^ uzi.lib

.PHONY: clean

clean: subclean
	rm -f uzi.nes link.map
