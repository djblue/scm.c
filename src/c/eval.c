#include <stdio.h>
#include <string.h>

#include "eval.h"
#include "print.h"

object_t *_if(vm_t *vm, object_t *predicate, object_t *consequent, object_t *alternative, object_t **env) {
  if (true(error(predicate))) return predicate;

  if (!false(predicate)) {
    return eval(vm, consequent, env);
  } else {
    return eval(vm, alternative, env);
  }
}

#define if_predicate(expr) car(vm, expr)
#define if_consequent(expr) car(vm, cdr(vm, expr))
#define if_alternative(expr) car(vm, cdr(vm, cdr(vm, expr)))

object_t *eval_if(vm_t *vm, object_t *expr, object_t **env) {

  object_t *body = cdr(vm, expr);

  return _if(vm, eval(vm, if_predicate(body), env), if_consequent(body), if_alternative(body), env);
}

object_t *eval_pair(vm_t *vm, object_t *expr, object_t **env) {

  object_t *ret = NULL;
  object_t *procedure = eval(vm, car(vm, expr), env);

  if (procedure == NULL) return make_error(vm, "nil is not operator");

  if (procedure->trace == 1) {
    printf("calling: ");
    print(vm, cons(vm, procedure, cdr(vm, expr)));
    printf("\n");
  }

  if (procedure->type == PRIMITIVE) {
    ret = prim_apply(vm, procedure, expr, env);
  } else if (procedure->type == PROCEDURE) {
    ret = proc_apply(vm, procedure, cdr(vm, expr), env);
  } else if (procedure->type == ERROR) {
    return procedure;
  } else {
    return make_error(vm, "not a procedure");
  }

  if (procedure->trace == 1) {
    printf("returning: ");
    print(vm, ret);
    printf("\n");
  }

  return ret;
}

object_t *eval_quote(vm_t *vm, object_t *expr, object_t **env) {
  return car(vm, cdr(vm, expr));
}

object_t *eval(vm_t *vm, object_t *expr, object_t **env) {
  if (expr == NULL) return NULL;

  switch (expr->type) {
    case SYMBOL:
      return lookup(vm, *env, expr);
    case PAIR:
      return eval_pair(vm, expr, env);
    default:
      return expr;
  }
}

object_t *eval_define(vm_t *vm, object_t *expr, object_t **env) {
  object_t *sym = car(vm, cdr(vm, expr));
  object_t *val = eval(vm, car(vm, cdr(vm, cdr(vm, expr))), env);
  define(vm, *env, sym, val);
  return val;
}

#define eval_predicate(fn,p) \
  object_t *fn(vm_t *vm, object_t *expr, object_t **env) { \
    object_t *o = eval(vm, car(vm, cdr(vm, expr)), env); \
    if (true(error(o))) return o; \
    return p(o); \
  }

eval_predicate(nullp, null)
eval_predicate(numberp, number)
eval_predicate(booleanp, boolean)
eval_predicate(errorp, error)
eval_predicate(stringp, string)
eval_predicate(characterp, character)
eval_predicate(symbolp, symbol)
eval_predicate(pairp, pair)

object_t *eval_lambda(vm_t *vm, object_t *expr, object_t **env) {
  object_t *args = car(vm, cdr(vm, expr));
  object_t *body = cdr(vm, cdr(vm, expr));
  return make_procedure(vm, *env, args, body);
}

object_t *trace(vm_t *vm, object_t *op) {
  op->trace = 1;
  return op;
}

// (trace operator)
object_t *eval_trace(vm_t *vm, object_t *expr, object_t **env) {
  object_t *op = eval(vm, car(vm, cdr(vm, expr)), env);
  return trace(vm, op);
}

object_t *untrace(vm_t *vm, object_t *op) {
  op->trace = 0;
  return op;
}

object_t *eval_untrace(vm_t *vm, object_t *expr, object_t **env) {
  object_t *op = eval(vm, car(vm, cdr(vm, expr)), env);
  return untrace(vm, op);
}

object_t *eval_eval(vm_t *vm, object_t *expr, object_t **env) {
  return eval(vm, eval(vm, car(vm, cdr(vm, expr)), env), env);
}

object_t *eval_cons(vm_t *vm, object_t *expr, object_t **env) {
  object_t *a = eval(vm, car(vm, cdr(vm, expr)), env);
  object_t *b = eval(vm, car(vm, cdr(vm, cdr(vm, expr))), env);
  return cons(vm, a, b);
}

object_t *eval_car(vm_t *vm, object_t *expr, object_t **env) {
  object_t *p = eval(vm, car(vm, cdr(vm, expr)), env);
  return car(vm, p);
}

object_t *eval_cdr(vm_t *vm, object_t *expr, object_t **env) {
  object_t *p = eval(vm, car(vm, cdr(vm, expr)), env);
  return cdr(vm, p);
}

object_t *eval_env(vm_t *vm, object_t *expr, object_t **env) {
  return *env;
}

object_t *eval_begin(vm_t *vm, object_t *expr, object_t **env) {
  object_t *val = eval(vm, car(vm, expr), env);
  object_t *next = cdr(vm, expr);

  if (next == NULL) {
    return val;
  } else {
    return eval_begin(vm, next, env);
  }
}

object_t *eval_print(vm_t *vm, object_t *expr, object_t **env) {
  object_t *o = eval(vm, car(vm, cdr(vm, expr)), env);
  print(vm, o);
  printf("\n");
  return o;
}

object_t *eval_eq(vm_t *vm, object_t *expr, object_t **env) {
  if (expr == NULL) return NULL;

  object_t *a = eval(vm, car(vm, cdr(vm, expr)), env);
  object_t *ls = cdr(vm, cdr(vm, expr));

  while (ls != NULL) {
    object_t *b = eval(vm, car(vm, ls), env);
    if (false(object_eq(vm, a, b))) return &f;
    ls = cdr(vm, ls);
  }

  return &t;
}

// (apply fn op1 ...)
object_t *eval_apply(vm_t *vm, object_t *expr, object_t **env) {
  return eval_pair(vm, cdr(vm, expr), env);
}

#include "number.h"

// (+ 1 2 3)
object_t *eval_plus(vm_t *vm, object_t *expr, object_t **env) {
  if (expr == NULL) return NULL;
  object_t *op = eval(vm, car(vm, cdr(vm, expr)), env);
  if (true(error(op))) return op;
  return plus(vm, op, eval_plus(vm, cdr(vm, expr), env));
}

object_t *eval_multiply(vm_t *vm, object_t *expr, object_t **env) {
  if (expr == NULL) return NULL;
  object_t *op = eval(vm, car(vm, cdr(vm, expr)), env);
  if (true(error(op))) return op;
  return multiply(vm, op, eval_multiply(vm, cdr(vm, expr), env));
}

void init(vm_t *vm, object_t *env) {
  def("+", eval_plus)
  def("*", eval_multiply)
  def("=", eval_eq)
  def("if", eval_if)
  def("quote", eval_quote)
  def("define", eval_define)
  def("apply", eval_apply)

  def("number?", numberp)
  def("boolean?", booleanp)
  def("error?", errorp)
  def("string?", stringp)
  def("char?", characterp)
  def("symbol?", symbolp)
  def("pair?", pairp)
  def("null?", nullp)

  def("eval", eval_eval)
  def("lambda", eval_lambda)
  def("trace", eval_trace)
  def("untrace", eval_untrace)

  def("cons", eval_cons)
  def("car", eval_car)
  def("cdr", eval_cdr)
  def("begin", eval_begin)

  def("write", eval_print)
  def("env", eval_env)
}
