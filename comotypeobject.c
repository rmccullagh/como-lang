#include "comotype2.h"

typedef struct {
		como_object_impl ob_base;
		long value;
} como_long_object;

como_type_spec ComoLong_Type = {
	{
		{ 1, NULL },
		0
	},
	"Como.Integer",
	sizeof(como_long_object),
	0,
	como_long_object_new,
	0
};

static como_object_impl *
como_long_object_new(como_type_spec *type, como_object_impl *args)
{
	como_long_object *self;
	self = (como_long_object *)type->tp_alloc(type, 0);
}

como_object_impl *como_init_long_module(void)
{
	como_object_impl *m;
	ComoLong_Type.tp_new = ComoType_GenericNew;

	if(ComoType_Ready(&ComoLong_Type) < 0)
		return NULL;

}

