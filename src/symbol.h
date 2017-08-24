#ifndef SYMBOL_H
#define SYMBOL_H

#include "object.h"
#include "string.h"
#include "vm.h"

object_t make_symbol(vm_t *vm, char *str);
object_t symbol(object_t o);

char *symbol_str(vm_t *vm, object_t sym);
int symbol_bound(object_t sym);
object_t symbol_get_binding(object_t sym);
void symbol_set_binding(object_t sym, object_t binding);
object_t symbol_eq(object_t a, object_t b);

#endif
