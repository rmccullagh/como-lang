#ifndef COMO_LONG_OBJECT_H
#define COMO_LONG_OBJECT_H

#include "comotype2.h"

typedef struct {
	como_object base;
  long value;
} ComoLong_Object;

extern comotypeobject ComoLong_Type;

ComoLong_Object *ComoLong_New(como_object *, como_object *);

#endif
