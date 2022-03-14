#include <stdio.h>
#include <stdlib.h>

int add(int a, int b)
{
	int sum;
	__asm__ volatile (
		"add %[result], %[val1], %[val2]"
		:[result]"=r"(sum)
		:[val1]"r"(a), [val2]"r"(b)
		:"cc"
	);
	return sum;
}

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

