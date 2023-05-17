CA := ca65
CC := cc65
LD := ld65

CAFLAGS := --include-dir include/asm

PLATFORM := uzi

TARGET := uzi.nes
LINKER_SCRIPT := uzi.cfg
KERNEL_OBJECTS := head.o kernel.o

.PRECIOUS: %.s %.o
.SUFFIXES:

all: $(TARGET)

%.s: %.c
	@echo "Building .c to .s $<"
	$(CC) --target $(PLATFORM) -I$(CC65_DIR)/include/ -I$(PWD)/include/ -O $<

%.o: %.S
	@echo "Building .S $<"
	$(CA) $(CAFLAGS) -o $@ $<

%.o: %.s
	@echo "Building .s $<"
	$(CA) -o $@ $<

$(TARGET): $(KERNEL_OBJECTS) $(LINKER_SCRIPT)
	$(LD) -v -m link.map -C $(LINKER_SCRIPT) -o $@ $(KERNEL_OBJECTS) uzi.lib

clean:
	rm -f *.o *.nes *.s

