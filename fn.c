#include <stdlib.h>
#include <stdio.h>

int func(int a) {
	return a + 1;
}

int main(void)
{
	void *fn = (void *)func;
	
	return 0;
}