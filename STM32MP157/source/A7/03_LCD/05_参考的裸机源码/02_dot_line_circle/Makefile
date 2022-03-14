PREFIX=arm-buildroot-linux-gnueabihf-
CC=$(PREFIX)gcc
LD=$(PREFIX)ld
AR=$(PREFIX)ar
OBJCOPY=$(PREFIX)objcopy
OBJDUMP=$(PREFIX)objdump

dot_line_circle.stm32 : start.S Stm32mp157_con.c main.c framebuffer.c geometry.c lcd_manager.c lcd_7_0.c lcd_controller_manager.c
	$(CC) -nostdlib -g -c -o start.o start.S
	$(CC) -nostdlib -g -c -o lcd_controller_manager.o lcd_controller_manager.c	
	$(CC) -nostdlib -g -c -o lcd_manager.o lcd_manager.c
	$(CC) -nostdlib -g -c -o lcd_7_0.o lcd_7_0.c
	$(CC) -nostdlib -g -c -o Stm32mp157_con.o Stm32mp157_con.c	
	$(CC) -nostdlib -g -c -o main.o main.c
	$(CC) -nostdlib -g -c -o framebuffer.o framebuffer.c	
	$(CC) -nostdlib -g -c -o geometry.o geometry.c
	
	$(LD) -T stm32mp157.lds -g start.o main.o geometry.o framebuffer.o lcd_controller_manager.o lcd_manager.o lcd_7_0.o Stm32mp157_con.o -o dot_line_circle.elf
	
	$(OBJCOPY) -O binary -S dot_line_circle.elf  dot_line_circle.bin
	$(OBJDUMP) -D -m arm  dot_line_circle.elf  > dot_line_circle.dis	
	./tools/mkimage -A arm -T firmware -C none -O u-boot -a 0xC0100000 -e 0 -d dot_line_circle.bin dot_line_circle.img >/dev/null
	./tools/mkimage -T stm32image -a 0xC0100000 -e 0xC0100000 -d dot_line_circle.bin dot_line_circle.stm32 >dot_line_circle.stm32.log

clean:
	rm -f dot_line_circle.dis  dot_line_circle.bin dot_line_circle.elf dot_line_circle.img dot_line_circle.stm32 *.o *.log


	
