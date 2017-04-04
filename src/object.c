#include <stdio.h>

#include "vm.h"
#include "types.h"

object_t make(vm_t *vm, type_t type, size_t n) {
  object_t o = (object_t) vm_alloc(vm, sizeof(object_t) + n);
  if (o == 0) {
    fprintf(stderr, "out of memory.");
    exit(1);
  }
  o->type = type;
  o->trace = 0;
  o->marked = 0;
  o->guard = 0;
  return o;
}

type_t scm_type(object_t o) {
  return o->type;
}

object_t object_eq(vm_t *vm, object_t a, object_t b) {
  if (a == b) return t;
  if (a == NULL || b == NULL) return f;
  if (scm_type(a) != scm_type(b)) return f;

  switch(scm_type(a)) {
    case FIXNUM:
      return number_eq(vm, a, b);
    case CHARACTER:
      return character_eq(vm, a, b);
    case SYMBOL:
      return symbol_eq(vm, a, b);
    case PAIR:
      return pair_eq(vm, a, b);
    default:
      return f;
  }
}

