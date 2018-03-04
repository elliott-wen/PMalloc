#include <malloc.h>
#include <stdio.h>

int main()
{
	printf("hello\n");
	char *a = malloc(1024);
	printf("%p\n", a);
}