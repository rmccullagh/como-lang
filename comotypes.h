#ifndef COMO_TYPES_H
#define COMO_TYPES_H

#include <object.h>
#include <stddef.h>

#define COMO_TYPE_IS_OBJECT (1 << 0)
#define COMO_TYPE_IS_FUNC   (1 << 1)
#define COMO_TYPE_IS_CALLABLE (1 << 2)
#define COMO_TYPE_IS_BUILTIN  (1 << 3)
#define COMO_TYPE_IS_READONLY (1 << 4)
#define COMO_TYPE_IS_SEALED (1 << 5)
#define COMO_TYPE_IS_CLASS  (1 << 6)
#define COMO_TYPE_IS_INSTANCE (1 << 7)

typedef struct como_object como_object;
typedef como_object *(*como_type_method)(como_object *, Object *);
typedef como_object *(*como_constructor_func)(como_object *, como_object *);
typedef como_object *(*comotypenewfn)(como_object *, como_object *);

typedef struct como_type {
	unsigned int flags;
	const char *name;
	Object *properties;
} como_type;

struct como_object {
	como_type   *type;	
	como_object *self;
	Object *value;
	unsigned int flags;
};

como_object *como_type_new_string_object(const char *);
como_object *como_type_new_int_object(long);
como_object *como_type_new_double_object(double);
como_object *como_type_new_function_object(como_object *, Object *);
como_object *como_type_new_undefined_object(void);
como_object *como_type_new_class(void);
como_object *como_type_new_instance(void);

#endif
