#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(void)
{
	const char *name = "Ryan McCullagh\\n";
	size_t len = strlen(name);

	char *buffer = malloc(len + 1);

	size_t i;
	for(i = 0; i < len; i++) {
		if(name[i] == '\\') {
			if(i + 1 < len) {
				char c = name[i + 1];
				switch(c) {
					case 'n':
						buffer[i] = '\n';					
					break;
					default:
						printf("invalid escape sequence in string literal at position %zu\n", i);
					break;
				}
				i = i + 1;
			}
		} else {
			buffer[i] = name[i];
		}
	}

	buffer[i] = '\0';

	printf("%zu\n", strlen(buffer));

	printf("%s", buffer);

	return 0;
}

