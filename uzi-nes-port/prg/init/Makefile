CA := ca65
CC := cc65
LD := ld65

.PRECIOUS: %.o
.SUFFIXES:

init: init.o
	$(LD) -v -m link.map -vm -C init.cfg -o $@ $^ uzi.lib

init.o: init.S init.cfg
	@echo "Building .S $<"
	$(CA) $(CA_INCLUDE) -o $@ $<

clean:
	rm *.o *.map init
