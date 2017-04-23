#ifndef BOOLEAN_H
#define BOOLEAN_H

#include "object.h"
#include "vm.h"

object_t make_boolean(vm_t *vm, char *str);

void define_boolean(vm_t *vm, object_t env);

#endif
