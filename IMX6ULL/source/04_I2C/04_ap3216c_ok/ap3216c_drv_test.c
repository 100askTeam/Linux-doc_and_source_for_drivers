
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

/*
 */
int main(int argc, char **argv)
{
	int fd;
	char buf[6];
	int len;
	

	/* 2. 打开文件 */
	fd = open("/dev/ap3216c", O_RDWR);
	if (fd == -1)
	{
		printf("can not open file /dev/hello\n");
		return -1;
	}

	len = read(fd, buf, 6);		
	printf("APP read : ");
	for (len = 0; len < 6; len++)
		printf("%02x ", buf[len]);
	printf("\n");
	
	close(fd);
	
	return 0;
}


