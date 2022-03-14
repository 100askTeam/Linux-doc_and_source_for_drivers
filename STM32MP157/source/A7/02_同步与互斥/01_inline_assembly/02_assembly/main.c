#include <stdio.h>
#include <stdlib.h>

extern int add(int a, int b);

int main(int argc, char **argv)
{
	int a;
	int b;
	
	if (argc != 3)
	{
		printf("Usage: %s <val1> <val2>\n", argv[0]);
		return -1;
	}

	a = (int)strtol(argv[1], NULL, 0);	
	b = (int)strtol(argv[2], NULL, 0);	

	printf("%d + %d = %d\n", a, b, add(a, b));
	return 0;
}

