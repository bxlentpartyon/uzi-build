CA := ca65
CC := cc65
LD := ld65

PLATFORM := uzi

TARGET := uzi.nes
LINKER_SCRIPT := scripts/linker/uzi.cfg
MAKEFILE_BUILD := $(PWD)/scripts/build/Makefile.build
LINKDEPS := link.d

subdirs := drivers fs include kernel lib ppu

FONT :=	font/newfont.chr

all: $(TARGET)

export AR CA CC LD PLATFORM MAKEFILE_BUILD LINKDEPS

-include $(MAKEFILE_BUILD)
-include $(LINKDEPS)

$(TARGET): $(O_DEPS)
	$(LD) -v -m link.map -vm -C $(LINKER_SCRIPT) -o $@ $^ uzi.lib

.PHONY: clean

clean: subclean
	rm -f uzi.nes link.map
