#include <stdio.h>
#include <string.h>

#include "eval.h"
#include "print.h"

object_t *eval_if(vm_t *vm, object_t *expr, object_t **env) {
  object_t *predicate = eval(vm, car(vm, expr), env);
  if (true(error(predicate))) return predicate;
  return eval(vm, !false(predicate) ? car(vm, cdr(vm, expr)) : car(vm, cdr(vm, cdr(vm, expr))), env);
}

object_t *eval_sequence(vm_t *vm, object_t *expr, object_t **env) {
  if (expr == NULL) return NULL;
  return cons(vm, eval(vm, car(vm, expr), env), eval_sequence(vm, cdr(vm, expr), env));
}

object_t *eval_pair(vm_t *vm, object_t *expr, object_t **env) {

  object_t *ret = NULL;
  object_t *procedure = eval(vm, car(vm, expr), env);
  object_t *args = cdr(vm, expr);

  if (procedure == NULL) return make_error(vm, "nil is not operator");

  if (procedure->trace == 1) {
    printf("calling: ");
    print(vm, cons(vm, procedure, expr));
    printf("\n");
  }

  if (procedure->type == SPECIAL) {
    ret = prim_apply(vm, procedure, args, env);
  } else if (procedure->type == PRIMITIVE) {
    ret = prim_apply(vm, procedure, eval_sequence(vm, args, env), env);
  } else if (procedure->type == PROCEDURE) {
    ret = proc_apply(vm, procedure, eval_sequence(vm, args, env), env);
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
  return car(vm, expr);
}

object_t *eval_unquote(vm_t *vm, object_t *expr, object_t **env) {
  if (expr == NULL || expr->type != PAIR) return expr;
  object_t *val = car(vm, expr);

  if (true(object_eq(vm, val, make_symbol(vm, "unquote")))) {
    return eval(vm, car(vm, cdr(vm, expr)), env);
  } else if (true(object_eq(vm, val, make_symbol(vm, "quasiquote")))) {
    return eval(vm, expr, env);
  }

  return cons(vm, eval_unquote(vm, val, env), eval_unquote(vm, cdr(vm, expr), env));
}

object_t *eval_quasiquote(vm_t *vm, object_t *expr, object_t **env) {
  return eval_unquote(vm, car(vm, expr), env);
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
  object_t *var = car(vm, expr);
  object_t *val = car(vm, cdr(vm, expr));

  if (true(pair(var))) {
    object_t *lambda = make_symbol(vm, "lambda");
    object_t *params = cdr(vm, var);
    val = cons(vm, lambda, cons(vm, params, cdr(vm, expr)));
    var = car(vm, var);
  }

  define(vm, *env, var, eval(vm, val, env));
  return &t;
}

object_t *eval_begin(vm_t *vm, object_t *expr, object_t **env) {
  object_t *val = eval(vm, car(vm, expr), env);
  object_t *next = cdr(vm, expr);
  if (next == NULL) return val;
  return eval_begin(vm, next, env);
}

object_t *eval_and(vm_t *vm, object_t *expr, object_t **env) {
  if (expr == NULL) return &t;
  object_t *val = eval(vm, car(vm, expr), env);
  object_t *next = cdr(vm, expr);
  if (next == NULL || false(val)) return val;
  return eval_and(vm, next, env);
}

object_t *eval_or(vm_t *vm, object_t *expr, object_t **env) {
  if (expr == NULL) return &f;
  object_t *val = eval(vm, car(vm, expr), env);
  object_t *next = cdr(vm, expr);
  if (next == NULL || !false(val)) return val;
  return eval_or(vm, next, env);
}

object_t *eval_cond(vm_t *vm, object_t *expr, object_t **env) {
  if (expr == NULL) return NULL;
  object_t *_case = car(vm, expr);
  object_t *test = car(vm, _case);
  object_t *body = car(vm, cdr(vm, _case));
  if (true(object_eq(vm, test, make_symbol(vm, "else"))) ||
      true(eval(vm, test, env))) {
    return eval(vm, body, env);
  }
  return eval_cond(vm, cdr(vm, expr), env);
}

#define eval_predicate(fn,p) \
  object_t *fn(vm_t *vm, object_t *expr, object_t **env) { \
    object_t *o = car(vm, expr); \
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
  object_t *args = car(vm, expr);
  object_t *body = cdr(vm, expr);
  return make_procedure(vm, *env, args, body);
}

object_t *trace(vm_t *vm, object_t *op) {
  op->trace = 1;
  return op;
}

// (trace operator)
object_t *eval_trace(vm_t *vm, object_t *expr, object_t **env) {
  object_t *op = car(vm, expr);
  return trace(vm, op);
}

object_t *untrace(vm_t *vm, object_t *op) {
  op->trace = 0;
  return op;
}

object_t *eval_untrace(vm_t *vm, object_t *expr, object_t **env) {
  object_t *op = car(vm, expr);
  return untrace(vm, op);
}

object_t *eval_eval(vm_t *vm, object_t *expr, object_t **env) {
  return eval(vm, car(vm, expr), env);
}

object_t *eval_cons(vm_t *vm, object_t *expr, object_t **env) {
  object_t *a = car(vm, expr);
  object_t *b = car(vm, cdr(vm, expr));
  return cons(vm, a, b);
}

object_t *eval_car(vm_t *vm, object_t *expr, object_t **env) {
  object_t *p = car(vm, expr);
  return car(vm, p);
}

object_t *eval_cdr(vm_t *vm, object_t *expr, object_t **env) {
  return cdr(vm, car(vm, expr));
}

object_t *eval_set_car(vm_t *vm, object_t *expr, object_t **env) {
  return set_car(vm, car(vm, expr), car(vm, cdr(vm, expr)));
}

object_t *eval_set_cdr(vm_t *vm, object_t *expr, object_t **env) {
  return set_cdr(vm, car(vm, expr), car(vm, cdr(vm, expr)));
}

object_t *eval_env(vm_t *vm, object_t *expr, object_t **env) {
  return *env;
}

object_t *eval_print(vm_t *vm, object_t *expr, object_t **env) {
  object_t *o = car(vm, expr);
  print(vm, o);
  return o;
}

object_t *eval_eq(vm_t *vm, object_t *expr, object_t **env) {
  if (expr == NULL) return NULL;

  object_t *a = car(vm, expr);
  object_t *ls = cdr(vm, expr);

  while (ls != NULL) {
    object_t *b = car(vm, ls);
    if (false(object_eq(vm, a, b))) return &f;
    ls = cdr(vm, ls);
  }

  return &t;
}

// (apply fn op1 ...)
object_t *eval_apply(vm_t *vm, object_t *expr, object_t **env) {
  object_t *op = car(vm, expr);
  object_t *args = car(vm, cdr(vm, expr));
  if (args != NULL && args->type != PAIR) {
    if (args->type == ERROR) {
      return args;
    } else {
      return make_error(vm, "cannot apply non-list arguments to function");
    }
  }
  return eval_pair(vm, cons(vm, op, args), env);
}

#include "number.h"

// (+ 1 2 3)
object_t *eval_plus(vm_t *vm, object_t *expr, object_t **env) {
  if (expr == NULL) return NULL;
  object_t *op = car(vm, expr);
  if (true(error(op))) return op;
  return plus(vm, op, eval_plus(vm, cdr(vm, expr), env));
}

object_t *eval_multiply(vm_t *vm, object_t *expr, object_t **env) {
  if (expr == NULL) return NULL;
  object_t *op = car(vm, expr);
  if (true(error(op))) return op;
  return multiply(vm, op, eval_multiply(vm, cdr(vm, expr), env));
}

void init(vm_t *vm, object_t *env) {

  // special forms
  defs("if", eval_if)
  defs("quote", eval_quote)
  defs("quasiquote", eval_quasiquote)
  defs("define", eval_define)
  defs("lambda", eval_lambda)
  defs("begin", eval_begin)
  defs("and", eval_and)
  defs("or", eval_or)
  defs("cond", eval_cond)

  def("+", eval_plus)
  def("*", eval_multiply)
  def("=", eval_eq)
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
  def("trace", eval_trace)
  def("untrace", eval_untrace)

  def("cons", eval_cons)
  def("car", eval_car)
  def("cdr", eval_cdr)
  def("set-car!", eval_set_car)
  def("set-cdr!", eval_set_cdr)

  def("write", eval_print)
  def("env", eval_env)
}

