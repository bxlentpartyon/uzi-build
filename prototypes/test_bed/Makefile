CA := ca65
CC := cc65
LD := ld65

CC65_DIR := $(UZIBUILD_ROOT)/cc65/

PLATFORM := nes

TARGET := test_bed.nes
LINKER_SCRIPT := test_bed.cfg

TARGET_DEPS := init.o lib.o libc.o main.o

.PRECIOUS: %.s %.o
.SUFFIXES:

all: $(TARGET)

clean:
	rm -f *.o *.nes *.s

%.s: %.c
	$(CC) --target $(PLATFORM) -I$(CC65_DIR)/include/ $<

%.o: %.s
	$(CA) $<

%.o: %.S
	$(CA) $<

$(TARGET): $(TARGET_DEPS) $(LINKER_SCRIPT)
	$(LD) -m link.map -C $(LINKER_SCRIPT) -o $@ $(TARGET_DEPS) $(PLATFORM).lib
