#ifndef COMO_TYPES_H
#define COMO_TYPES_H

#include <object.h>
#include <stddef.h>

#define COMO_TYPE_IS_OBJECT (1 << 0)
#define COMO_TYPE_IS_FUNC   (1 << 1)
#define COMO_TYPE_IS_CALLABLE (1 << 2)
#define COMO_TYPE_IS_READONLY (1 << 3)
#define COMO_TYPE_IS_SEALED (1 << 4)
#define COMO_TYPE_IS_CLASS  (1 << 5)

typedef struct como_object como_object;
typedef como_object *(*como_type_method)(como_object *, Object *);
typedef como_object *(*como_constructor_func)(como_object *, como_object *);
typedef como_object *(*comotypenewfn)(como_object *, como_object *);

typedef struct _comotypeobject {
	const char    *name;
	ssize_t        size;
	unsigned long  flags;
  comotypenewfn  newfunc;
  Object        *properties; 
} comotypeobject;

typedef struct como_type {
	unsigned int flags;
	const char *name;
	como_constructor_func ctor;
	Object *properties;
} como_type;

struct como_object {
	como_type   *type;	
	como_object *self;
	Object *value;
	unsigned int flags;
};

como_object *como_type_new_error_object(void);
como_object *como_type_init_string(const char *);
como_object *como_type_new_int_object(long);
como_object *como_type_new_double_object(double);
como_object *como_type_new_function_object(como_object *, Object *);
como_object *como_type_new_undefined_object(void);

#endif
