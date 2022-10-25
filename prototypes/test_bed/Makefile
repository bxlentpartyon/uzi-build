CA := ca65
LD := ld65

PLATFORM := nes

TARGET := test_bed.nes
LINKER_SCRIPT := test_bed.cfg

TARGET_DEPS := init.o lib.o

.PRECIOUS: %.s %.o

all: $(TARGET)

clean:
	rm -f *.o *.nes

%.o: %.s
	$(CA) $<

$(TARGET): $(TARGET_DEPS) $(LINKER_SCRIPT)
	$(LD) -C $(LINKER_SCRIPT) -o $@ $(TARGET_DEPS)
