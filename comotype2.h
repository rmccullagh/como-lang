#ifndef COMO_TYPE_OBJECT_H
#define COMO_TYPE_OBJECT_H

#include <object.h>
#include <sys/types.h>

#define COMO_TYPE_IS_OBJECT   (1 << 0)
#define COMO_TYPE_IS_CALLABLE (1 << 2)
#define COMO_TYPE_IS_READONLY (1 << 3)
#define COMO_TYPE_IS_SEALED   (1 << 4)
#define COMO_TYPE_IS_CLASS    (1 << 5)

typedef struct _como_object como_object;
typedef struct _comotypeobject comotypeobject;
typedef como_object *(*comotypenewfn)(comotypeobject *, como_object *);
typedef como_object *(*comomethodfn)(como_object *, como_object *);

struct _comotypeobject {
	const char    *name;
	ssize_t        size;
	unsigned long  flags;
  comotypenewfn  newfunc;
  Object        *properties; 
};

struct _como_object {
    ssize_t refcount;
    struct _comotypeobject *type;	
};

#endif
