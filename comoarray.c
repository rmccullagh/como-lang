#include <stdlib.h>
#include <object.h>

#include "comotype2.h"
#include "comoarray.h"

static como_object *
object_push(como_object *self, como_object *args)
{
	ComoArray_Object *base = (ComoArray_Object *)self;
	if(args == NULL) {
		return NULL;		
	}

	if(args->type == &ComoLong_Type) {
		ComoLong_Object *lo = (ComoLong_Object *)args;
		arrayPush(base->array, newLong(lo->value));
	}
}

static como_object *
object_at(como_object *self, como_object *args)
{
	return NULL;
}

static como_object *
object_count(como_object *self, como_object *args)
{
	return NULL;
}

static como_object *
object_new(comotypeobject *type, como_object *args)
{
	ComoArray_Object *self = malloc(sizeof(ComoArray_Object));
	self->base.refcount = 1;
  self->base.type = &ComoArray_Type;
  self->array = newArray(8);
  self->push = array_push;
	self->at = array_at;
	self->count = array_count;
	return (como_object *)self;
}

comotypeobject ComoArray_Type = {
	"Array",
   sizeof(ComoArray_Object),
   0,
   array_new,
   0
};

