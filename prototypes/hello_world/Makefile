CA := ca65
LD := ld65

PLATFORM := nes

TARGETS := hello_world.nes
LINKER_SCRIPT := hello_world.cfg

.PRECIOUS: %.s %.o

all: $(TARGETS)

clean:
	rm -f *.o *.nes

%.o: %.S
	$(CA) $<

%.nes: %.o $(LINKER_SCRIPT)
	$(LD) -C $(LINKER_SCRIPT) -o $@ $<
