#ifndef SYMBOL_H
#define SYMBOL_H

#include "object.h"
#include "string.h"
#include "vm.h"

object_t *make_symbol(vm_t *vm, char *str);
object_t *symbol(object_t *o);

object_t *symbol_eq(vm_t *vm, object_t *a, object_t *b);

#endif
