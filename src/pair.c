#include <stdarg.h>

#include "pair.h"
#include "error.h"
#include "boolean.h"
#include "eval.h"

typedef struct {
  object_t car;
  object_t cdr;
} pair_t;

object_t cons(vm_t *vm, object_t car, object_t cdr) {
  object_t o = make(vm, PAIR, sizeof(pair_t));
  object_data(o, pair_t).car = car;
  object_data(o, pair_t).cdr = cdr;
  return o;
}

object_t car(object_t pair) {
  if (pair == NULL) return NULL;
  return object_data(pair, pair_t).car;
}

object_t cdr(object_t pair) {
  if (pair == NULL) return NULL;
  return object_data(pair, pair_t).cdr;
}

object_t set_car(object_t pair, object_t car) {
  if (pair == NULL) return NULL;
  object_data(pair, pair_t).car = car;
  return t;
}

object_t set_cdr(object_t pair, object_t cdr) {
  if (pair == NULL) return NULL;
  object_data(pair, pair_t).cdr = cdr;
  return t;
}

object_t list(vm_t *vm, int argc, ...) {
  va_list argp;
  object_t head = NULL, *prev = NULL;

  va_start(argp, argc);

  for (int i = 0; i < argc; i++) {
    object_t pair = cons(vm, va_arg(argp, object_t), NULL);
    if (prev != NULL) {
      set_cdr(prev, pair);
    } else if (head == NULL) {
      head = pair;
    }
    prev = pair;
  }

  va_end(argp);

  return head;
}

object_t null(object_t o) {
  if (o == NULL) return t;
  return f;
}

predicate(pair, PAIR)

object_t pair_eq(vm_t *vm, object_t a, object_t b) {
  return false(object_eq(vm, car(a), car(b))) ? f : object_eq(vm, cdr(a), cdr(b));
}

#define eval_predicate(fn,p) \
  object_t fn(vm_t *vm, object_t args) { \
    object_t o = car(args); \
    return p(o); \
  }

eval_predicate(nullp, null)
eval_predicate(pairp, pair)

object_t eval_cons(vm_t *vm, object_t args) {
  return cons(vm, car(args), cadr(args));
}

object_t eval_car(vm_t *vm, object_t args) {
  object_t pair = car(args);
  if (pair == NULL) return NULL;
  if (scm_type(pair) != PAIR) {
    return make_error(vm, "car: object not pair", pair);
  }
  return car(pair);
}

object_t eval_cdr(vm_t *vm, object_t args) {
  object_t pair = car(args);
  if (pair == NULL) return NULL;
  if (scm_type(pair) != PAIR) {
    return make_error(vm, "cdr: object not pair", pair);
  }
  return cdr(pair);
}

object_t eval_set_car(vm_t *vm, object_t args) {
  object_t pair = car(args);
  if (pair == NULL) return NULL;
  if (scm_type(pair) != PAIR) {
    return make_error(vm, "set-car! object not pair", pair);
  }
  return set_car(pair, cadr(args));
}

object_t eval_set_cdr(vm_t *vm, object_t args) {
  object_t pair = car(args);
  if (pair == NULL) return NULL;
  if (scm_type(pair) != PAIR) {
    return make_error(vm, "set-cdr! object not pair", pair);
  }
  return set_cdr(pair, cadr(args));
}

void define_pair(vm_t *vm, object_t env) {
  def("cons", eval_cons)
  def("car", eval_car)
  def("cdr", eval_cdr)
  def("set-car!", eval_set_car)
  def("set-cdr!", eval_set_cdr)

  def("pair?", pairp)
  def("null?", nullp)
}
