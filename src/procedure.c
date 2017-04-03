#include "procedure.h"
#include "pair.h"
#include "env.h"
#include "eval.h"

#include "print.h"

object_t make_procedure(vm_t *vm, object_t env, object_t params, object_t body) {
  object_t o = make(vm, PROCEDURE, sizeof(proc_t));
  object_data(o, proc_t).body = body;
  object_data(o, proc_t).params = params;
  object_data(o, proc_t).env = env;
  return o;
}

predicate(procedure, PROCEDURE)
