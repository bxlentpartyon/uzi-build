CA := ca65
LD := ld65

PLATFORM := nes

TARGETS := hello_mmc1.nes
LINKER_SCRIPT := hello_mmc1.cfg

.PRECIOUS: %.s %.o

all: $(TARGETS)

clean:
	rm -f *.o *.nes *.s

%.o: %.S
	$(CA) $<

%.nes: %.o $(LINKER_SCRIPT)
	$(LD) -C $(LINKER_SCRIPT) -o $@ $<
