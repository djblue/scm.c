#include "procedure.h"
#include "pair.h"
#include "env.h"
#include "eval.h"

#include "print.h"

object_t make_procedure(vm_t *vm, object_t env, object_t params, object_t body, int macro) {
  object_t o = make(vm, macro ? MACRO : PROCEDURE, sizeof(proc_t));
  object_data(o, proc_t).body = body;
  object_data(o, proc_t).params = params;
  object_data(o, proc_t).env = env;
  object_data(o, proc_t).macro = macro;
  return o;
}

object_t procedure(object_t o) {
  return scm_type(o) != PROCEDURE ? f : t;
}
