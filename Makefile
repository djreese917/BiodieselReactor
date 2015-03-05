program_5: blink.c
	avr-gcc -mmcu=atmega328p -DF_CPU=16000000 -O2 -o blink.elf blink.c
	avr-objcopy -O ihex blink.elf blink.hex
	avr-size blink.elf
				  
#Flash the Arduino
#Be sure to change the device (the argument after -P) to match the device on your  computer
#On Windows, change the argument after -P to appropriate COM port
program: blink.hex
	avrdude -pm328p -P /dev/ttyACM0 -c arduino -F -u -U flash:w:blink.hex
							  
#remove build files
clean:
	rm -fr *.elf *.hex *.o

