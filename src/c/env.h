#ifndef ENV_H
#define ENV_H

#include "object.h"

object_t *make_frame(vm_t *vm, object_t *parent);

#define def(sym,fun) \
  define(env, make_symbol(sym), make_primitive(fun));

object_t *define(vm_t *vm, object_t *env, object_t *sym, object_t *val);
object_t *lookup(vm_t *vm, object_t *env, object_t *sym);

#endif
