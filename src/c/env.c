#include "types.h"

object_t *make_frame(vm_t *vm, object_t *parent) {
  return cons(vm, NULL, parent);
}

object_t *define(vm_t *vm, object_t *frame, object_t *sym, object_t *val) {
  set_car(vm, frame, cons(vm, cons(sym, val), car(frame)));
  return frame;
}

object_t *frame_search(vm_t *vm, object_t *frame, object_t *sym) {
  if (frame == NULL || sym == NULL) return make_error(vm, "cannot find symbol");
  object_t *pair = car(vm, frame);
  if (true(symbol_eq(vm, car(vm, pair), sym))) return cdr(vm, pair);
  return frame_search(vm, cdr(vm, frame), sym);
}

object_t *lookup(vm_t *vm, object_t *env, object_t *sym) {
  if (sym == NULL) return NULL;
  if (env == NULL) return make_error(vm, "cannot find symbol");
  object_t *find = frame_search(vm, car(vm, env), sym);
  if (find == NULL || false(error(vm, find))) return find;
  return lookup(vm, cdr(vm, env), sym);
}

