#ifndef CHARACTER_H
#define CHARACTER_H

#include "object.h"
#include "vm.h"

object_t *make_char(vm_t *vm, char *str);

object_t *character(object_t *o);
object_t *character_eq(vm_t *vm, object_t *a, object_t *b);

#endif
