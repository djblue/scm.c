#include <stdio.h>

#include "vm.h"
#include "types.h"
#include "number.h"

struct _object_t {
  unsigned char type;
  unsigned char trace;
  unsigned char marked;
  unsigned char guard;
};

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
  if (o == NULL) return NIL;
  if (scm_is_fixnum(o)) return FIXNUM;
  return o->type;
}

void scm_mark(object_t o) {
  if (scm_is_fixnum(o)) return;
  o->marked = 1;
}

void scm_unmark(object_t o) {
  if (scm_is_fixnum(o)) return;
  o->marked = 0;
}

object_t scm_guard(object_t o) {
  if (scm_is_fixnum(o)) return o;
  o->guard = 1;
  return o;
}

object_t scm_unguard(object_t o) {
  if (scm_is_fixnum(o)) return o;
  o->guard = 0;
  return o;
}

int scm_has_guard(object_t o) {
  if (scm_is_fixnum(o)) return 1;
  return o->guard;
}

int scm_is_marked(object_t o) {
  if (scm_is_fixnum(o)) return 1;
  return o->marked;
}

object_t object_eq(object_t a, object_t b) {
  if (a == b) return t;
  if (a == NULL || b == NULL) return f;
  if (scm_type(a) != scm_type(b)) return f;

  switch(scm_type(a)) {
    case FIXNUM:
      return number_eq(a, b);
    case CHARACTER:
      return character_eq(a, b);
    case SYMBOL:
      return symbol_eq(a, b);
    case PAIR:
      return pair_eq(a, b);
    default:
      return f;
  }
}

