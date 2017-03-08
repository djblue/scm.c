#ifndef CHARACTER_H
#define CHARACTER_H

#include "object.h"

object_t *make_char(vm_t *vm, char *str);

object_t *character(vm_t *vm, object_t *o);
object_t *character_eq(vm_t *vm, object_t *a, object_t *b);

#endif
