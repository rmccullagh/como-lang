#include <stdio.h>
#include <stdlib.h>
#include "comotype2.h"
#include "comolongobject.h"

static como_object *
como_object_new(comotypeobject *type, como_object *inst, como_object *args)
{
	(void)inst;
	(void)args;

	if(type->newfunc == NULL)
		return NULL;

	return type->newfunc(type, NULL);
}


int main(void)
{
	
	como_object *instance = como_object_new(&ComoLong_Type, NULL, NULL);

	if(instance->type == &ComoLong_Type) {
		printf("instance.type is ComoLong_Type\n");
		printf("%s\n", instance->type->name);
		ComoLong_Object *lo = (ComoLong_Object *)instance;
		printf("%ld\n", lo->value);
	}	

	como_object *baseobject = como_object_new(&ComoObject_Type, NULL, NULL);

	if(baseobject->type == &ComoObject_Type) {
		printf("baseobject.type is ComoObject_Type\n");
		printf("%s\n", baseobject->type->name);
		ComoObject_Object *oo = (ComoObject_Object *)baseobject;
		printf("%ld\n", lo->value);
	}	

	return 0;
}
