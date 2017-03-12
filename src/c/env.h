#ifndef ENV_H
#define ENV_H

#include "object.h"
#include "vm.h"

object_t *extend_frame(vm_t *vm, object_t *vars, object_t *vals, object_t *parent);
object_t *make_frame(vm_t *vm);

#define def(sym,fun) \
  define(vm, env, make_symbol(vm, sym), make_primitive(vm, fun, 0));

#define defs(sym,fun) \
  define(vm, env, make_symbol(vm, sym), make_primitive(vm, fun, 1));

object_t *define(vm_t *vm, object_t *env, object_t *sym, object_t *val);
object_t *lookup(vm_t *vm, object_t *env, object_t *sym);

#endif
