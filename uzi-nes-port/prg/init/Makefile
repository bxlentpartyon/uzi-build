CA := ca65
CC := cc65
LD := ld65

PLATFORM := uzi

TARGET := uzi.nes

LINKER_SCRIPT := init.cfg

init.s: init.c
	@echo "Building init.c to init.s $<"
	$(CC) --target $(PLATFORM) -I$(CC65_DIR)/include/ -O $<

init.o: init.s
	@echo "Building init.o $<"
	$(CA) -o $@ $<

init.rom:  $(LINKER_SCRIPT) $(FONT)
	$(LD) -v -m link.map -C $(LINKER_SCRIPT) -o $@ $(ALL_DEPS) uzi.lib
