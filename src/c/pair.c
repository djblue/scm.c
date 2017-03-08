#include "pair.h"
#include "error.h"
#include "boolean.h"
#include "eval.h"

typedef struct {
  object_t *car;
  object_t *cdr;
} pair_t;

object_t *cons(vm_t *vm, object_t *car, object_t *cdr) {
  object_t *o = make(vm, PAIR, sizeof(pair_t));
  object_data(o, pair_t).car = car;
  object_data(o, pair_t).cdr = cdr;
  return o;
}

object_t *car(vm_t *vm, object_t *pair) {
  if (pair == NULL) return NULL;
  if (pair->type == ERROR) return pair;
  if (pair->type != PAIR) {
    return make_error(vm, "object not pair");
  }
  return object_data(pair, pair_t).car;
}

object_t *cdr(vm_t *vm, object_t *pair) {
  if (pair == NULL) return NULL;
  if (pair->type == ERROR) return pair;
  if (pair->type != PAIR) {
    return make_error(vm, "object not pair");
  }
  return object_data(pair, pair_t).cdr;
}

void set_car(vm_t *vm, object_t *pair, object_t *car) {
  object_data(pair, pair_t).car = car;
}

void set_cdr(vm_t *vm, object_t *pair, object_t *cdr) {
  object_data(pair, pair_t).cdr = cdr;
}

object_t *null(vm_t *vm, object_t *o) {
  if (o == NULL) return &t;
  return &f;
}

predicate(pair, PAIR)

object_t *pair_eq(vm_t *vm, object_t *a, object_t *b) {
  return false(object_eq(vm, car(vm, a), car(b))) ? &f : object_eq(cdr(vm, a), cdr(b));
}

