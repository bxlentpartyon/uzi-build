CA := ca65
CC := cc65
LD := ld65

PLATFORM := nes

TARGET := test_bed.nes
LINKER_SCRIPT := test_bed.cfg

TARGET_DEPS := init.o lib.o main.o

.PRECIOUS: %.s %.o

all: $(TARGET)

clean:
	rm -f *.o *.nes

%.o: %.c
%.s: %.c
	$(CC) --target $(PLATFORM) $<

%.o: %.s
	$(CA) $<

$(TARGET): $(TARGET_DEPS) $(LINKER_SCRIPT)
	$(LD) -C $(LINKER_SCRIPT) -o $@ $(TARGET_DEPS)
