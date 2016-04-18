#ifndef COMO_BASE_OBJECT_H
#define COMO_BASE_OBJECT_H

#include <object.h>
#include "comotype2.h"

typedef struct {
	como_object base;
	Object *dict;
	comomethodfn set;
	comomethodfn get;
} ComoObject_Object;

extern comotypeobject ComoObject_Type;

#endif
