
.PHONY: clean flash dump_fuses

SRC=main.c
PART=t13
MCU=attiny13

CFLAGS+=-Os -std=c99 -Wall -Wextra

prog.hex: prog.elf
	avr-objcopy -j .text -j .data -O ihex $< $@

prog.elf: $(SRC)
	avr-gcc -mmcu=$(MCU) $(CFLAGS) -o $@ $^

# -B sets programmer speed in microseconds. if AVR is running at 128 kHz we need it
.flash_timestamp: prog.hex
	avrdude -p $(PART) -e -U flash:w:prog.hex -B 100
	touch $@

flash: .flash_timestamp ;

dump_fuses:
	avrdude -p $(PART) -U lfuse:r:lfuse:h -B 1000
	avrdude -p $(PART) -U hfuse:r:hfuse:h -B 1000
	cat lfuse hfuse

clean:
	rm -f prog.hex prog.elf lfuse hfuse

