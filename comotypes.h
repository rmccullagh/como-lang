#ifndef COMO_TYPES_H
#define COMO_TYPES_H

typedef struct como_object como_object;
typedef como_object *(*como_type_method)(como_object *, Object *);

typedef struct como_type {
	unsigned int flags;
	const char *name;
	Object *properties;
} como_type;

struct como_object {
	como_type *type;	
	Object *value;
	unsigned int flags;
};

#endif
