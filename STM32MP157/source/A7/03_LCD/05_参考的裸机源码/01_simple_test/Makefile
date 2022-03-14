PREFIX=arm-buildroot-linux-gnueabihf-
CC=$(PREFIX)gcc
LD=$(PREFIX)ld
AR=$(PREFIX)ar
OBJCOPY=$(PREFIX)objcopy
OBJDUMP=$(PREFIX)objdump

simple_test.stm32 : start.S Stm32mp157_con.c main.c lcd_manager.c lcd_7_0.c lcd_controller_manager.c
	$(CC) -nostdlib -g -c -o start.o start.S
	$(CC) -nostdlib -g -c -o lcd_controller_manager.o lcd_controller_manager.c	
	$(CC) -nostdlib -g -c -o lcd_manager.o lcd_manager.c
	$(CC) -nostdlib -g -c -o lcd_7_0.o lcd_7_0.c
	$(CC) -nostdlib -g -c -o Stm32mp157_con.o Stm32mp157_con.c	
	$(CC) -nostdlib -g -c -o main.o main.c
	
	$(LD) -T stm32mp157.lds -g start.o main.o lcd_controller_manager.o lcd_manager.o lcd_7_0.o Stm32mp157_con.o -o simple_test.elf
	
	$(OBJCOPY) -O binary -S simple_test.elf  simple_test.bin
	$(OBJDUMP) -D -m arm  simple_test.elf  > simple_test.dis	
	./tools/mkimage -A arm -T firmware -C none -O u-boot -a 0xC0100000 -e 0 -d simple_test.bin simple_test.img >/dev/null
	./tools/mkimage -T stm32image -a 0xC0100000 -e 0xC0100000 -d simple_test.bin simple_test.stm32 >simple_test.stm32.log

clean:
	rm -f simple_test.dis  simple_test.bin simple_test.elf simple_test.img simple_test.stm32 *.o *.log


	
