#include <stdio.h>
#include <string.h>

#include "eval.h"
#include "print.h"

object_t *eval_sequence(vm_t *vm, object_t *expr, object_t *env) {
  if (expr == NULL || expr->type == ERROR) return expr;
  return cons(vm, eval(vm, car(vm, expr), env), eval_sequence(vm, cdr(vm, expr), env));
}

object_t *eval(vm_t *vm, object_t *expr, object_t *env) {
tailcall:
  if (expr == NULL) return NULL;
  if (expr->type == SYMBOL) return lookup(vm, env, expr);
  if (expr->type != PAIR) return expr;

  object_t *procedure = eval(vm, car(vm, expr), env);
  object_t *args = cdr(vm, expr);

  if (procedure == NULL) return make_error(vm, "nil is not operator");

  if (procedure->type == SPECIAL) {
    expr = args;

    switch (object_data(procedure, special_t)) {
      case F_IF: {
        object_t *predicate = eval(vm, car(vm, expr), env);
        if (true(error(predicate))) return predicate;
        expr = !false(predicate) ? car(vm, cdr(vm, expr)) : car(vm, cdr(vm, cdr(vm, expr)));
        goto tailcall;
      }
      case F_QUOTE: {
        return car(vm, expr);
      }
      case F_DEFINE: {
        object_t *var = car(vm, expr);
        object_t *val = car(vm, cdr(vm, expr));

        if (true(pair(var))) {
          object_t *params = cdr(vm, var);
          val = cons(vm, sym_lambda, cons(vm, params, cdr(vm, expr)));
          var = car(vm, var);
        }

        define(vm, env, var, eval(vm, val, env));
        return &t;
      }
      case F_LAMBDA: {
        object_t *args = car(vm, expr);
        object_t *body = cdr(vm, expr);
        return make_procedure(vm, env, args, body);
      }
      case F_BEGIN: {
        if (expr == NULL) return NULL;
        while (cdr(vm, expr) != NULL) {
          eval(vm, car(vm, expr), env);
          expr = cdr(vm, expr);
        }
        expr = car(vm, expr);
        goto tailcall;
      }
      case F_AND: {
        if (expr == NULL) return &t;

        while (cdr(vm, expr) != NULL) {
          object_t *val = eval(vm, car(vm, expr), env);
          if (false(val)) return val;
          expr = cdr(vm, expr);
        }
        expr = car(vm, expr);
        goto tailcall;
      }
      case F_OR: {
        if (expr == NULL) return &f;

        while (cdr(vm, expr) != NULL) {
          object_t *val = eval(vm, car(vm, expr), env);
          if (!false(val)) return val;
          expr = cdr(vm, expr);
        }
        expr = car(vm, expr);
        goto tailcall;
      }
      case F_COND: {
        while (expr != NULL) {
          object_t *_case = car(vm, expr);
          object_t *test = car(vm, _case);
          object_t *body = car(vm, cdr(vm, _case));
          if (true(object_eq(vm, test, make_symbol(vm, "else"))) ||
              true(eval(vm, test, env))) {
            expr = body;
            goto tailcall;
          }
          expr = cdr(vm, expr);
        }
        goto tailcall;
      }
      case F_EVAL: {
        expr = eval(vm, car(vm, expr), env);
        goto tailcall;
      }
      default: return make_error(vm, "oh no!!!");
    }
  } else if (procedure->type == PRIMITIVE) {
    return prim_apply(vm, procedure, eval_sequence(vm, args, env), env);
  } else if (procedure->type == PROCEDURE) {
    object_t *body = cons(vm, sym_begin, object_data(procedure, proc_t).body);
    object_t *parent = object_data(procedure, proc_t).env; // captured environment
    object_t *params = object_data(procedure, proc_t).params;

    object_t *vars = params;
    object_t *vals = eval_sequence(vm, args, env);

    if (true(symbol(vars))) {
      vars = cons(vm, vars, NULL);
      vals = cons(vm, vals, NULL);
    }

    env = extend_frame(vm, vars, vals, parent);
    expr = body;
    goto tailcall;
  } else if (procedure->type == ERROR) {
    return procedure;
  } else {
    return make_error(vm, "not a procedure");
  }
}

#define eval_predicate(fn,p) \
  object_t *fn(vm_t *vm, object_t *args) { \
    object_t *o = car(vm, args); \
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

object_t *eval_cons(vm_t *vm, object_t *args) {
  object_t *a = car(vm, args);
  object_t *b = car(vm, cdr(vm, args));
  return cons(vm, a, b);
}

object_t *eval_car(vm_t *vm, object_t *args) {
  object_t *p = car(vm, args);
  return car(vm, p);
}

object_t *eval_cdr(vm_t *vm, object_t *args) {
  return cdr(vm, car(vm, args));
}

object_t *eval_set_car(vm_t *vm, object_t *args) {
  return set_car(vm, car(vm, args), car(vm, cdr(vm, args)));
}

object_t *eval_set_cdr(vm_t *vm, object_t *args) {
  return set_cdr(vm, car(vm, args), car(vm, cdr(vm, args)));
}

object_t *eval_print(vm_t *vm, object_t *args) {
  object_t *o = car(vm, args);
  print(vm, o);
  return o;
}

object_t *eval_eq(vm_t *vm, object_t *args) {
  if (args == NULL) return NULL;

  object_t *a = car(vm, args);
  object_t *ls = cdr(vm, args);

  while (ls != NULL) {
    object_t *b = car(vm, ls);
    if (false(object_eq(vm, a, b))) return &f;
    ls = cdr(vm, ls);
  }

  return &t;
}

#include "number.h"

// (+ 1 2 3)
object_t *eval_plus(vm_t *vm, object_t *args) {
  if (args == NULL) return NULL;
  object_t *op = car(vm, args);
  if (true(error(op))) return op;
  return plus(vm, op, eval_plus(vm, cdr(vm, args)));
}

object_t *eval_multiply(vm_t *vm, object_t *args) {
  if (args == NULL) return NULL;
  object_t *op = car(vm, args);
  if (true(error(op))) return op;
  return multiply(vm, op, eval_multiply(vm, cdr(vm, args)));
}

void init(vm_t *vm, object_t *env) {

  // special forms
  sym_if = defs("if", F_IF)
  sym_quote = defs("quote", F_QUOTE)
  sym_lambda = defs("lambda", F_LAMBDA)
  sym_begin = defs("begin", F_BEGIN)
  sym_and = defs("and", F_AND)
  sym_or = defs("or", F_OR)
  sym_cond = defs("cond", F_COND)
  sym_define = defs("define", F_DEFINE)
  sym_eval = defs("eval", F_EVAL)

  def("+", eval_plus)
  def("*", eval_multiply)
  def("=", eval_eq)

  def("number?", numberp)
  def("boolean?", booleanp)
  def("error?", errorp)
  def("string?", stringp)
  def("char?", characterp)
  def("symbol?", symbolp)
  def("pair?", pairp)
  def("null?", nullp)

  def("cons", eval_cons)
  def("car", eval_car)
  def("cdr", eval_cdr)
  def("set-car!", eval_set_car)
  def("set-cdr!", eval_set_cdr)

  def("write", eval_print)
}

