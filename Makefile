objects=hello.o rfid.o spi.o uart.o
elf=hello.elf
hex=hello.hex
mmcu=atmega328p
port=/dev/ttyACM0

build: compile
	avr-objcopy -j .data -j .text -O ihex $(elf) $(hex)
	sudo avrdude -P $(port) -c arduino -p m328p -U flash:w:$(hex)

compile: $(objects)
	avr-gcc -Os -mmcu=$(mmcu) -o $(elf) $(objects)

$(objects): %.o : %.cpp %.h
	avr-gcc -g -Os -mmcu=$(mmcu) -I/usr/lib/avr/include -c $< -o $@

hello.h:

.PHONY: compile build
