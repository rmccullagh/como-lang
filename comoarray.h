#ifndef COMO_ARRAY_OBJECT_H
#define COMO_ARRAY_OBJECT_H

#include <object.h>
#include "comotype2.h"

typedef struct {
	como_object base;
	Object *array;
	comomethodfn push;
	comomethodfn at;
	comomethodfn count;	
} ComoArray_Object;

extern comotypeobject ComoArray_Type;

#endif
