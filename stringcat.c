#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <object.h>

static
Object* MY_stringCat(Object *o1, Object *o2)
{
	String *s1 = O_SVAL(o1);
	String *s2 = O_SVAL(o2);

	size_t len = s1->length + s2->length + 1;

	char *buffer = malloc(len);

	if(!buffer)
		return NULL;	

	memcpy(buffer, s1->value, s1->length);
	memcpy(buffer + s1->length, s2->value, s2->length);

	buffer[len - 1] = '\0';

	Object *retval = newString(buffer);

	if(!retval) {
		free(buffer);
		return NULL;
	}
	
	free(buffer);

	return retval;
}

int main(void) {

	Object *first = newString("Ryan");
	object_print_stats(first);
	Object *last = newString(" McCullagh");
	object_print_stats(last);
	Object *name = MY_stringCat(first, last);

	object_print_stats(name);

	objectDestroy(first);
	objectDestroy(last);

	objectDestroy(name);

	return 0;
}