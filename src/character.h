#ifndef CHARACTER_H
#define CHARACTER_H

#include "object.h"
#include "vm.h"

object_t make_char(vm_t *vm, char *str);
object_t _make_char(vm_t *vm, char c);

object_t character_eq(vm_t *vm, object_t a, object_t b);

void define_char(vm_t *vm, object_t env);

#endif
