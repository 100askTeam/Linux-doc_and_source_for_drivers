#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

/*
 * dac_test /dev/mydac <val>
 */

int main(int argc, char **argv)
{
	int fd;
	int buf[2];
	unsigned short dac_val = 0;

	if (argc != 3)
	{
		printf("Usage: %s <dev> <val>\n", argv[0]);
		return -1;
	}
	
	fd = open(argv[1], O_RDWR);
	if (fd < 0)
	{
		printf(" can not open %s\n", argv[1]);
		return -1;
	}

	dac_val = strtoul(argv[2], NULL, 0);

	//while (1)
	{
		write(fd, &dac_val, 2);
	//	dac_val += 50;
	}
	
	return 0;
}


