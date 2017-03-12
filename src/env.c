#include "types.h"

// An environment is model as a list of pairs of vars and values.
//
// eg: (((x y z) . (4 5 6))   ; the closest frame
//      ...
//      (() . ())             ; an empty frame
//      ...
//      ((a b c) . (1 2 3)))  ; the furthest frame
//
//
object_t *extend_frame(vm_t *vm, object_t *vars, object_t *vals, object_t *parent) {
  return cons(vm, cons(vm, vars, vals), parent);
}

object_t *make_frame(vm_t *vm) {
  return extend_frame(vm, NULL, NULL, NULL);
}

object_t *define(vm_t *vm, object_t *frames, object_t *var, object_t *val) {
  object_t *frame = car(vm, frames);

  object_t *vars = car(vm, frame);
  object_t *vals = cdr(vm, frame);

  set_car(vm, frame, cons(vm, var, vars));
  set_cdr(vm, frame, cons(vm, val, vals));

  return frame;
}

static object_t *frame_search(vm_t *vm, object_t *frame, object_t *sym) {
  object_t *vars = car(vm, frame);
  object_t *vals = cdr(vm, frame);

  while (vars != NULL) {
    if (true(symbol_eq(vm, car(vm, vars), sym)))
      return car(vm, vals);

    vars = cdr(vm, vars);
    vals = cdr(vm, vals);
  }

  return make_error(vm, "cannot find symbol");
}

object_t *lookup(vm_t *vm, object_t *env, object_t *sym) {
  if (sym == NULL) return NULL;

  while (env != NULL) {
    object_t *found = frame_search(vm, car(vm, env), sym);
    if (found == NULL || false(error(found))) return found;
    env = cdr(vm, env);
  }

  return make_error(vm, "cannot find symbol");
}

