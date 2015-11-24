#include <stdio.h>
#include <stdlib.h>

#define GET_IP() 1

int main(void)
{

	for(;;) {
		switch(GET_IP()) {
			case BAIL_OUT:
				goto done;
			break;
		}
	}


	done:
		printf("Finishing exeuction context succesfully\n");

		return 0;
}
