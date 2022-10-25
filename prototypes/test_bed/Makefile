CA := ca65
LD := ld65

PLATFORM := nes

TARGETS := hello_mmc5.nes
LINKER_SCRIPT := hello_mmc5.cfg

.PRECIOUS: %.s %.o

all: $(TARGETS)

clean:
	rm -f *.o *.nes

%.o: %.s
	$(CA) $<

%.nes: %.o $(LINKER_SCRIPT)
	$(LD) -C $(LINKER_SCRIPT) -o $@ $<
