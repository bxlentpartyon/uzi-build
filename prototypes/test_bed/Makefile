CA := ca65
LD := ld65

PLATFORM := nes

TARGETS := test_bed.nes
LINKER_SCRIPT := test_bed.cfg

.PRECIOUS: %.s %.o

all: $(TARGETS)

clean:
	rm -f *.o *.nes

%.o: %.s
	$(CA) $<

%.nes: %.o $(LINKER_SCRIPT)
	$(LD) -C $(LINKER_SCRIPT) -o $@ $<
