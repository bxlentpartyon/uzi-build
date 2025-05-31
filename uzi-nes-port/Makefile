AR := ar65
CA := ca65
CC := cc65
LD := ld65

PLATFORM := uzi

TARGET := uzi.nes
LINKER_SCRIPT := scripts/linker/uzi.cfg
MAKEFILE_BUILD := $(PWD)/scripts/build/Makefile.build

subdirs := drivers fs include kernel lib ppu

FONT :=	font/newfont.chr

all: $(TARGET)

export AR CA CC LD PLATFORM MAKEFILE_BUILD

-include $(MAKEFILE_BUILD)
-include deps.d

$(TARGET): $(O_DEPS)
	$(LD) -v -m link.map -vm -C $(LINKER_SCRIPT) -o $@ $^ uzi.lib

.PHONY: clean

clean: subclean
	rm -f uzi.nes link.map
