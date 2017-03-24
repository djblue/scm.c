#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include "object.h"
#include "vm.h"

object_t *sym_if;
object_t *sym_quote;
object_t *sym_define;
object_t *sym_lambda;
object_t *sym_begin;
object_t *sym_and;
object_t *sym_or;
object_t *sym_cond;
object_t *sym_eval;

typedef enum {
  F_IF,
  F_QUOTE,
  F_DEFINE,
  F_LAMBDA,
  F_BEGIN,
  F_AND,
  F_OR,
  F_COND,
  F_EVAL
} special_t;

typedef struct object_t *(*primitive)(vm_t *vm, object_t *expr, object_t *env);

#define defn(name) static object_t *name(vm_t *vm, object_t *expr, object_t *env)

object_t *make_special(vm_t *vm, special_t s);
object_t *make_primitive(vm_t *vm, primitive fn);

object_t *prim_apply(vm_t *vm, object_t *prim, object_t *args, object_t *env);

#endif
