#include "procedure.h"
#include "pair.h"
#include "env.h"
#include "eval.h"

#include "print.h"

object_t *make_procedure(vm_t *vm, object_t *env, object_t *params, object_t *body) {
  object_t *o = make(vm, PROCEDURE, sizeof(proc_t));
  object_data(o, proc_t).body = body;
  object_data(o, proc_t).params = params;
  object_data(o, proc_t).env = env;
  return o;
}

predicate(procedure, PROCEDURE)

object_t *eval_args(vm_t *vm, object_t *frame, object_t *params, object_t *args, object_t **env) {
  if (args == NULL || params == NULL) return frame;

  object_t *sym = car(vm, params);
  object_t *val = car(vm, args);

  return define(vm, eval_args(vm, frame, cdr(vm, params), cdr(vm, args), env), sym, val);
}

object_t *proc_apply(vm_t *vm, object_t *procedure, object_t *args, object_t **__remove) {
  object_t *begin = make_symbol(vm, "begin");

  object_t *body = cons(vm, begin, object_data(procedure, proc_t).body);
  object_t *parent = object_data(procedure, proc_t).env; // captured environment
  object_t *params = object_data(procedure, proc_t).params;

  object_t *vars = params;
  object_t *vals = args;

  if (true(symbol(vars))) {
    vars = cons(vm, vars, NULL);
    vals = cons(vm, vals, NULL);
  }

  object_t *env = extend_frame(vm, vars, vals, parent);

  return eval(vm, body, &env);
}
