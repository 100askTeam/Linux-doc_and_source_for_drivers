CROSS=arm-linux-gnueabihf-

test: main.c
	$(CROSS)gcc -o $@ $^
	$(CROSS)objdump -D $@ > $@.dis
clean:
	rm test
	
