#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include "object.h"
#include "vm.h"

object_t sym_if;
object_t sym_quote;
object_t sym_define;
object_t sym_lambda;
object_t sym_begin;
object_t sym_and;
object_t sym_or;
object_t sym_cond;
object_t sym_case;
object_t sym_eval;

object_t eof;
object_t ueof;
object_t t;
object_t f;

object_t sym_else;

typedef enum {
  F_IF,
  F_QUOTE,
  F_DEFINE,
  F_LAMBDA,
  F_BEGIN,
  F_AND,
  F_OR,
  F_COND,
  F_CASE,
  F_EVAL
} special_t;

typedef object_t (*primitive)(vm_t *vm, object_t args);

object_t make_special(vm_t *vm, special_t s);
object_t make_primitive(vm_t *vm, primitive fn);

#endif
