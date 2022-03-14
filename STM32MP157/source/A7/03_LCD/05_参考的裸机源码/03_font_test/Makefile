PREFIX=arm-buildroot-linux-gnueabihf-
CC=$(PREFIX)gcc
LD=$(PREFIX)ld
AR=$(PREFIX)ar
OBJCOPY=$(PREFIX)objcopy
OBJDUMP=$(PREFIX)objdump

font_test.stm32 : start.S Stm32mp157_con.c main.c font.c font_8x16.c framebuffer.c geometry.c lcd_manager.c lcd_7_0.c lcd_controller_manager.c
	$(CC) -nostdlib -g -c -o start.o start.S
	$(CC) -nostdlib -g -c -o lcd_controller_manager.o lcd_controller_manager.c	
	$(CC) -nostdlib -g -c -o lcd_manager.o lcd_manager.c
	$(CC) -nostdlib -g -c -o lcd_7_0.o lcd_7_0.c
	$(CC) -nostdlib -g -c -o Stm32mp157_con.o Stm32mp157_con.c	
	$(CC) -nostdlib -g -c -o main.o main.c
	$(CC) -nostdlib -g -c -o framebuffer.o framebuffer.c	
	$(CC) -nostdlib -g -c -o geometry.o geometry.c
	$(CC) -nostdlib -g -c -o font.o font.c
	$(CC) -nostdlib -g -c -o font_8x16.o font_8x16.c
	
	$(LD) -T stm32mp157.lds -g start.o main.o font_8x16.o font.o geometry.o framebuffer.o lcd_controller_manager.o lcd_manager.o lcd_7_0.o Stm32mp157_con.o -o font_test.elf
	
	$(OBJCOPY) -O binary -S font_test.elf  font_test.bin
	$(OBJDUMP) -D -m arm  font_test.elf  > font_test.dis	
	./tools/mkimage -A arm -T firmware -C none -O u-boot -a 0xC0100000 -e 0 -d font_test.bin font_test.img >/dev/null
	./tools/mkimage -T stm32image -a 0xC0100000 -e 0xC0100000 -d font_test.bin font_test.stm32 >font_test.stm32.log

clean:
	rm -f font_test.dis  font_test.bin font_test.elf font_test.img font_test.stm32 *.o *.log


	
