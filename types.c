#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

typedef struct como_string_object como_string_object;
typedef struct como_long_object como_long_object;
typedef struct como_type como_type;
typedef ssize_t (*como_type_print_func)(como_type *);

struct como_type {
	como_type_print_func ct_print;
};

struct como_long_object {
	struct como_type type;
	long value;
};

static ssize_t como_long_object_print(como_type *o)
{
	como_long_object *clo;
	ssize_t retval;

	clo = (como_long_object *)o;

	retval = fprintf(stdout, "%ld\n", clo->value);
	
	return retval;
}

como_type *como_type_create_long_object(long value)
{
	como_long_object *o = malloc(sizeof(como_long_object));
	o->type.ct_print = como_long_object_print;
	o->value = value;
	return (como_type *)o;
}

struct como_string_object {
	struct como_type type;
	size_t len;
	char   *value;
};

static ssize_t como_string_object_print(como_type *o)
{
	como_string_object *cso;
	ssize_t retval;

	cso = (como_string_object *)o;

	retval = fprintf(stdout, "%s (%zu)\n", cso->value, cso->len);
	
	return retval;
}

como_type *como_type_create_string_object(char *value)
{
	como_string_object *o = malloc(sizeof(como_string_object));
	o->type.ct_print = como_string_object_print;
	o->len = strlen(value);
	o->value = malloc(o->len + 1);
	memcpy(o->value, value, o->len);
	o->value[o->len] = '\0';
	return (como_type *)o;
}

int main(void)
{
	como_type *o = como_type_create_long_object(1553);

	o->ct_print(o);

	como_type *o2 = como_type_create_string_object("Ryan");

	o2->ct_print(o2);

	return 0;
}





