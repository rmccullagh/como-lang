#include <object.h>
#include <stdlib.h>
#include <ctype.h>

#include "comotypes.h"

static como_object *como_type_string_length(como_object *self, Object *args)
{
	return como_type_new_int_object(O_SVAL(self->value)->length);
}

static como_object *como_type_string_touppercase(como_object *self, Object *args)
{
	const char *sval = O_SVAL(self->value)->value;
	const size_t slen = O_SVAL(self->value)->length;
	como_object *retval = NULL;
	char *tmpstr = malloc(slen + 1);

	size_t i;
	for(i = 0; i < slen; i++) {
		int c = (int)sval[i];
		if(isprint(c)) {
			tmpstr[i] = (char)toupper(c);
		} else {
			tmpstr[i] = (char)c;
		}
	}
	tmpstr[slen] = '\0';

	retval = como_type_init_string(tmpstr);
	free(tmpstr);
	return retval;
}

como_object *como_type_init_string(char *sval)
{
	como_object *o = malloc(sizeof(como_object));
	o->self = o;
	o->value = newString(sval == NULL ? "" : sval);
	o->flags = 0;
	o->type = malloc(sizeof(como_type));
	o->type->name = "String";
	o->type->flags = 0;
	o->type->properties = newMap(2);
	Object *length = newFunction(como_type_string_length);
	O_MRKD(length) = COMO_TYPE_IS_FUNC;
	mapInsert(o->type->properties, "length", length);
	objectDestroy(length);

	Object *toUpperCase = newFunction(como_type_string_touppercase);
	O_MRKD(toUpperCase) = COMO_TYPE_IS_FUNC;
	mapInsert(o->type->properties, "toUpperCase", toUpperCase);
	objectDestroy(toUpperCase);

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
	o->self = o;
	o->value = newLong(lval);
	o->flags = 0;
	o->type = malloc(sizeof(como_type));
	o->type->name = "Int";
	o->type->properties = newMap(2);
	o->type->flags = 0;
	Object *toString = newFunction(como_type_int_to_string);
	O_MRKD(toString) = COMO_TYPE_IS_FUNC;
	mapInsert(o->type->properties, "toString", toString);
	objectDestroy(toString);
	return o;
}

como_object *como_type_new_double_object(double dval)
{
	como_object *o = malloc(sizeof(como_object));
	o->self = o;
	o->value = newDouble(dval);
	o->flags = 0;
	o->type = malloc(sizeof(como_type));
	o->type->name = "Double";
	o->type->properties = newMap(2);
	o->type->flags = 0;
	Object *toString = newFunction(como_type_int_to_string);
	O_MRKD(toString) = COMO_TYPE_IS_FUNC;
	mapInsert(o->type->properties, "toString", toString);
	objectDestroy(toString);
	return o;
}

como_object *como_type_new_function_object(como_object *context, 
		Object *fval)
{
	como_object *o = malloc(sizeof(como_object));
	o->self = context;
	o->value = fval;
	o->flags = COMO_TYPE_IS_CALLABLE;
	o->type = malloc(sizeof(como_type));
	o->type->name = "Function";
	o->type->properties = newMap(2);
	o->type->flags = 0;
	return o;
}








