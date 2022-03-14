CROSS=arm-linux-gnueabihf-

test: main.c add.S
	$(CROSS)gcc -o $@ $^
	$(CROSS)objdump -D $@ > $@.dis
clean:
	rm test
	
