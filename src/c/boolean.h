#ifndef BOOLEAN_H
#define BOOLEAN_H

#include "object.h"
#include "vm.h"

object_t *make_boolean(vm_t *vm, char *str);

object_t *boolean(object_t *o);

#endif
