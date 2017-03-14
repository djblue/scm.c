#include <stdio.h>

#include "vm.h"
#include "types.h"

object_t *make(vm_t *vm, type_t type, size_t n) {
  object_t *o = (object_t*) vm_alloc(vm, sizeof(object_t) + n);
  if (o == 0) {
    fprintf(stderr, "out of memory.");
    exit(1);
  }
  o->type = type;
  o->trace = 0;
  o->marked = 0;
  o->padding = 0;
  return o;
}

object_t *object_eq(vm_t *vm, object_t *a, object_t *b) {
  if (a == b) return &t;
  if (a == NULL || b == NULL) return &f;
  if (a->type != b->type) return &f;

  switch(a->type) {
    case FIXNUM:
      return number_eq(vm, a, b);
    case CHARACTER:
      return character_eq(vm, a, b);
    case SYMBOL:
      return symbol_eq(vm, a, b);
    case PAIR:
      return pair_eq(vm, a, b);
  }

  return &f;
}

