#ifndef BOOLEAN_H
#define BOOLEAN_H

#include "object.h"

object_t *make_boolean(vm_t *vm, char *str);

object_t *boolean(vm_t *vm, object_t *o);

#endif
