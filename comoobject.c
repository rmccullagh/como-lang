#include <stdlib.h>
#include <object.h>

#include "comotype2.h"
#include "comoobject.h"

static como_object *
object_set(como_object *self, como_object *args)
{
	return NULL;
}

static como_object *
object_get(como_object *self, como_object *args)
{
	return NULL;
}

static como_object *
object_new(comotypeobject *type, como_object *args)
{
	ComoObject_Object *self = malloc(sizeof(ComoObject_Object));
	self->base.refcount = 1;
  self->base.type = &ComoObject_Type;
  self->dict = newMap(8);
	self->set = object_set;
	self->get = object_get; 
  return (como_object *)self;
}

comotypeobject ComoObject_Type = {
	"Object",
   sizeof(ComoObject_Object),
   0,
   object_new,
   0
};

