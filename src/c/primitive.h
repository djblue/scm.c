#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include "object.h"

typedef struct object_t *(*primitive)(object_t *expr, object_t **env);

#define defn(name) static object_t *name(object_t *expr, object_t **env)

object_t *make_primitive(vm_t *vm, primitive fn);

object_t *prim_apply(vm_t *vm, object_t *prim, object_t *args, object_t **env);

#endif
