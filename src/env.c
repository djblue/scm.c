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
object_t extend_frame(vm_t *vm, object_t vars, object_t vals, object_t parent) {
  return cons(vm, cons(vm, vars, vals), parent);
}

object_t make_frame(vm_t *vm) {
  return extend_frame(vm, NULL, NULL, NULL);
}

static object_t frame_search(vm_t *vm, object_t frame, object_t sym) {
  object_t vars = car(frame);
  object_t vals = cdr(frame);

  while (vars != NULL) {
    if (true(symbol_eq(car(vars), sym))) return vals;

    vars = cdr(vars);
    vals = cdr(vals);
  }

  return NULL;
}

object_t define(vm_t *vm, object_t frames, object_t var, object_t val) {
  symbol_set_binding(var, val);

  object_t frame = car(frames);
  object_t pair = frame_search(vm, frame, var);

  if (pair != NULL) {
    set_car(pair, val);
  } else {
    object_t vars = car(frame);
    object_t vals = cdr(frame);

    set_car(frame, cons(vm, var, vars));
    set_cdr(frame, cons(vm, val, vals));
  }

  return var;
}

object_t set(vm_t *vm, object_t env, object_t sym, object_t val) {
  if (sym == NULL) return NULL;

  while (env != NULL) {
    object_t frame = car(env);

    object_t vars = car(frame);
    object_t vals = cdr(frame);

    while (vars != NULL) {
      if (scm_type(vars) == SYMBOL && true(symbol_eq(vars, sym))) {
        set_car(vals, val);
        return t;
      } else if (true(symbol_eq(car(vars), sym))) {
        set_car(vals, val);
        return t;
      }

      vars = cdr(vars);
      vals = cdr(vals);
    }

    env = cdr(env);
  }

  return make_error(vm, "set!: cannot find var", sym);
}

object_t lookup(vm_t *vm, object_t env, object_t sym) {
  if (sym == NULL) return NULL;

  while (env != NULL) {
    object_t frame = car(env);

    object_t vars = car(frame);
    object_t vals = cdr(frame);

    while (vars != NULL) {
      if (scm_type(vars) == SYMBOL && true(symbol_eq(vars, sym))) {
        return vals;
      } else if (true(symbol_eq(car(vars), sym))) {
        return car(vals);
      }

      vars = cdr(vars);
      vals = cdr(vals);
    }

    env = cdr(env);
  }

  return make_error(vm, "lookup: no such binding", sym);
}

