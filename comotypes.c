#include <object.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#include "comotypes.h"

como_object *como_type_new_string_object(const char *sval)
{
	como_object *o = malloc(sizeof(como_object));
	o->self = o;
	o->value = newString(sval == NULL ? "" : sval);
	o->flags = 0;
	o->type = malloc(sizeof(como_type));
	o->type->name = "String";
	o->type->flags = 0;
	o->type->properties = newMap(2);
	return o;
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

como_object *como_type_new_undefined_object(void)
{
	como_object *o = malloc(sizeof(como_object));
	o->self = o;
	o->value = newNull();
	o->flags = COMO_TYPE_IS_OBJECT;
	o->type = malloc(sizeof(como_type));
	o->type->name = "Undefined";
	o->type->properties = newMap(2);
	o->type->flags = 0;
	return o;
}

como_object *como_type_new_class(void)
{
	como_object *o = malloc(sizeof(como_object));
	o->self = o;
	o->value = newNull();
	o->flags = COMO_TYPE_IS_OBJECT|COMO_TYPE_IS_CLASS;
	o->type = malloc(sizeof(como_type));
	o->type->name = "Class";
	o->type->properties = NULL;
	o->type->flags = 0;
	return o;
}

como_object *como_type_new_instance(void)
{
	como_object *o = malloc(sizeof(como_object));
	o->self = o;
	o->value = newNull();
	o->flags = COMO_TYPE_IS_OBJECT|COMO_TYPE_IS_INSTANCE;
	o->type = malloc(sizeof(como_type));
	o->type->name = "Instance";
	o->type->properties = NULL;
	o->type->flags = 0;
	return o;
}


