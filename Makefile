filenames=hello rfid spi uart
objects=$(patsubst %, $(builddir)/%.o, $(filenames))
builddir=build
elf=$(builddir)/hello.elf
hex=$(builddir)/hello.hex
mmcu=atmega328p
port=/dev/ttyACM0

build: compile
	avr-objcopy -j .data -j .text -O ihex $(elf) $(hex)
	sudo avrdude -P $(port) -c arduino -p m328p -U flash:w:$(hex)

compile: createbuilddir $(objects)
	avr-gcc -Os -mmcu=$(mmcu) -o $(elf) $(objects)

$(objects): $(builddir)/%.o : %.cpp %.h
	avr-gcc -g -Os -mmcu=$(mmcu) -I/usr/lib/avr/include -c $< -o $@

createbuilddir:
	mkdir -p ./build

hello.h:

.PHONY: compile build createbuilddir
