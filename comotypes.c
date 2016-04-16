#include <object.h>
#include <stdlib.h>

#include "comotypes.h"

static como_object *como_type_string_length(como_object *self, Object *args)
{
	return como_type_new_int_object(O_LVAL(self->value));
}

como_object *como_type_init_string(char *sval)
{
	como_object *o = malloc(sizeof(como_object));
	o->value = newString(sval == NULL ? "" : sval);
	o->flags = 0;
	o->type = malloc(sizeof(como_type));
	o->type->name = "String";
	o->type->flags = 0;
	o->type->properties = newMap(2);
	Object *length = newFunction(como_type_string_length);
	mapInsert(o->type->properties, "length", length);
	objectDestroy(length);
	return o;
}

static como_object *como_type_int_to_string(como_object *self, Object *args)
{
	char *s = objectToString(self->value);
	como_object *retval = como_type_init_string(s);
	free(s);
	return retval;
}

como_object *como_type_new_int_object(long lval)
{
	como_object *o = malloc(sizeof(como_object));
	o->value = newLong(lval);
	o->flags = 0;
	o->type = malloc(sizeof(como_type));
	o->type->name = "Int";
	o->type->properties = newMap(2);
	Object *toString = newFunction(como_type_int_to_string);
	o->type->flags = 0;
	mapInsert(o->type->properties, "toString", toString);
	objectDestroy(toString);
	return o;
}


