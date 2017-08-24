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

static object_t nullp(vm_t *vm, size_t n, object_t args[]) {
  if (n != 1) {
    return make_error(vm, "null?: incorrect argument count", NULL);
  }

  return null(args[0]);
}

static object_t pairp(vm_t *vm, size_t n, object_t args[]) {
  if (n != 1) {
    return make_error(vm, "pair?: incorrect argument count", NULL);
  }

  return pair(args[0]);
}

object_t eval_cons(vm_t *vm, size_t n, object_t args[]) {
  if (n != 2) {
    return make_error(vm, "cons: incorrect argument count", NULL);
  }

  return cons(vm, args[0], args[1]);
}

object_t eval_car(vm_t *vm, size_t n, object_t args[]) {
  if (n != 1) {
    return make_error(vm, "car: incorrect argument count", NULL);
  }

  object_t pair = args[0];

  if (scm_type(pair) != PAIR) {
    return make_error(vm, "car: object is not pair", pair);
  }

  return car(pair);
}

object_t eval_cdr(vm_t *vm, size_t n, object_t args[]) {
  if (n != 1) {
    return make_error(vm, "cdr: incorrect argument count", NULL);
  }

  object_t pair = args[0];

  if (scm_type(pair) != PAIR) {
    return make_error(vm, "cdr: object is not pair", pair);
  }

  return cdr(pair);
}

object_t eval_set_car(vm_t *vm, size_t n, object_t args[]) {
  if (n != 2) {
    return make_error(vm, "set-car!: incorrect argument count", NULL);
  }

  object_t pair = args[0];

  if (scm_type(pair) != PAIR) {
    return make_error(vm, "set-car!: object not pair", pair);
  }

  return set_car(pair, args[1]);
}

object_t eval_set_cdr(vm_t *vm, size_t n, object_t args[]) {
  if (n != 2) {
    return make_error(vm, "set-cdr!: incorrect argument count", NULL);
  }

  object_t pair = args[0];

  if (scm_type(pair) != PAIR) {
    return make_error(vm, "set-cdr!: object not pair", pair);
  }

  return set_cdr(pair, args[1]);
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
