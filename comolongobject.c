#include <stdlib.h>

#include "comotype2.h"
#include "comolongobject.h"

static como_object *
long_new(comotypeobject *type, como_object *args)
{
	ComoLong_Object *self = malloc(sizeof(ComoLong_Object));
	self->base.refcount = 1;
  self->base.type = &ComoLong_Type;
  self->value = 0;
  return (como_object *)self;
}

ComoLong_Object *ComoLong_New(como_object *type, como_object *t)
{
	ComoLong_Object *self = malloc(sizeof(ComoLong_Object));
	self->base.refcount = 1;
  self->base.type = &ComoLong_Type;
  self->value = 0;
  return self;
}

comotypeobject ComoLong_Type = {
	"Int",
   sizeof(ComoLong_Object),
   0,
   long_new,
   0
};