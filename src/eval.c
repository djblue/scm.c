#include <stdio.h>
#include <string.h>

#include "eval.h"
#include "print.h"

#define SAVE \
  push(vm, fetch(vm, EXPR)); \
  push(vm, fetch(vm, ENV)); \

#define RESTORE \
  assign(vm, ENV, pop(vm)); \
  assign(vm, EXPR, pop(vm));

object_t *eval_sequence(vm_t *vm) {
  size_t count = 0;

  while (fetch(vm, EXPR) != NULL) {
    SAVE
    assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
    object_t *value = eval(vm);
    RESTORE
    push(vm, value);
    count++;
    assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));
  }

  return popn(vm, count);
}

object_t *eval(vm_t *vm) {
tailcall:
  vm_gc(vm);

  if (fetch(vm, EXPR) == NULL) return NULL;
  if (fetch(vm, EXPR)->type == SYMBOL) return lookup(vm, fetch(vm, ENV), fetch(vm, EXPR));
  if (fetch(vm, EXPR)->type != PAIR) return fetch(vm, EXPR);

  push(vm, fetch(vm, EXPR));
  push(vm, fetch(vm, ENV));
  assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
  object_t *procedure = eval(vm);
  assign(vm, ENV, pop(vm));
  assign(vm, EXPR, pop(vm));

  if (procedure == NULL) return make_error(vm, "nil is not operator");

  assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));

  if (procedure->type == SPECIAL) {
    switch (object_data(procedure, special_t)) {
      case F_IF: {
        SAVE
        assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
        object_t *predicate = eval(vm);
        RESTORE
        if (true(error(predicate))) return predicate;
        assign(vm, EXPR, !false(predicate)
            ? car(vm, cdr(vm, fetch(vm, EXPR)))
            : car(vm, cdr(vm, cdr(vm, fetch(vm, EXPR)))));
        goto tailcall;
      }

      case F_QUOTE:
        return car(vm, fetch(vm, EXPR));

      case F_DEFINE: {
        object_t *var = car(vm, fetch(vm, EXPR));
        object_t *val = car(vm, cdr(vm, fetch(vm, EXPR)));

        if (true(pair(var))) {
          object_t *params = cdr(vm, var);
          val = cons(vm, sym_lambda, cons(vm, params, cdr(vm, fetch(vm, EXPR))));
          var = car(vm, var);
        }

        SAVE
        assign(vm, EXPR, val);
        object_t *result = eval(vm);
        RESTORE

        define(vm, fetch(vm, ENV), var, result);
        return &t;
      }

      case F_LAMBDA:
        return make_procedure(vm,
            fetch(vm, ENV),
            car(vm, fetch(vm, EXPR)),
            cdr(vm, fetch(vm, EXPR)));

      case F_BEGIN:
        if (fetch(vm, EXPR) == NULL) return NULL;
        while (cdr(vm, fetch(vm, EXPR)) != NULL) {

          SAVE
          assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
          eval(vm);
          RESTORE

          assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));
        }
        assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
        goto tailcall;

      case F_AND:
        if (fetch(vm, EXPR) == NULL) return &t;
        while (cdr(vm, fetch(vm, EXPR)) != NULL) {

          SAVE
          assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
          object_t *val = eval(vm);
          RESTORE

          if (false(val)) return val;
          assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));
        }
        assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
        goto tailcall;

      case F_OR:
        if (fetch(vm, EXPR) == NULL) return &f;
        while (cdr(vm, fetch(vm, EXPR)) != NULL) {

          SAVE
          assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
          object_t *val = eval(vm);
          RESTORE

          if (!false(val)) return val;
          assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));
        }
        assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
        goto tailcall;

      case F_COND:
        while (fetch(vm, EXPR) != NULL) {
          object_t *_case = car(vm, fetch(vm, EXPR));
          object_t *test = car(vm, _case);
          object_t *body = car(vm, cdr(vm, _case));
          if (true(object_eq(vm, test, make_symbol(vm, "else")))) {
            assign(vm, EXPR, body);
            goto tailcall;
          }

          SAVE
          assign(vm, EXPR, test);
          object_t *val = eval(vm);
          RESTORE

          if (true(val)) {
            assign(vm, EXPR, body);
            goto tailcall;
          }

          assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));
        }
        goto tailcall;

      case F_EVAL:
        push(vm, fetch(vm, ENV));
        assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
        assign(vm, EXPR, eval(vm));
        assign(vm, ENV, pop(vm));
        goto tailcall;

      default: return make_error(vm, "oh no!!!");
    }
  } else if (procedure->type == PRIMITIVE) {
    return (object_data(procedure, primitive))(vm, eval_sequence(vm));
  } else if (procedure->type == PROCEDURE) {

    SAVE
    object_t *vals = eval_sequence(vm);
    RESTORE

    object_t *body = cons(vm, sym_begin, object_data(procedure, proc_t).body);
    object_t *parent = object_data(procedure, proc_t).env; // captured environment
    object_t *params = object_data(procedure, proc_t).params;

    object_t *vars = params;

    if (true(symbol(vars))) {
      vars = cons(vm, vars, NULL);
      vals = cons(vm, vals, NULL);
    }

    assign(vm, ENV, extend_frame(vm, vars, vals, parent));
    assign(vm, EXPR, body);

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

