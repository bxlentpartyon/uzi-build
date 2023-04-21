CA := ca65
LD := ld65

PLATFORM := nes

TARGET := uzi.nes
LINKER_SCRIPT := uzi.cfg
KERNEL_OBJECTS := head.o

.PRECIOUS: %.s %.o

all: $(TARGET)

%.s: %.c
	@echo "Building .c to .s $<"
	$(CC) --target $(PLATFORM) -I$(CC65_DIR)/include/ -I$(PWD)/include/ -O $<

%.o: %.S
	$(CA) $<

%.o: %.s
	@echo "Building .s $<"
	$(CA) $<

$(TARGET): $(KERNEL_OBJECTS) $(LINKER_SCRIPT)
	$(LD) -C $(LINKER_SCRIPT) -o $@ $<

clean:
	rm -f *.o *.nes *.s

