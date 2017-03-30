#pragma GCC diagnostic ignored "-Wpedantic"

#include <stdio.h>
#include <string.h>

#include "eval.h"
#include "print.h"

#define SAVE \
  push(vm, fetch(vm, EXPR)); \
  push(vm, fetch(vm, ENV)); \
  push(vm, fetch(vm, FUN)); \
  push(vm, fetch(vm, CONTINUE)); \
  push(vm, fetch(vm, ARGL));

#define RESTORE \
  assign(vm, ARGL, pop(vm)); \
  assign(vm, CONTINUE, pop(vm)); \
  assign(vm, FUN, pop(vm)); \
  assign(vm, ENV, pop(vm)); \
  assign(vm, EXPR, pop(vm));


#define RET(value)do{\
  assign(vm, VAL, (value)); \
  if (fetch(vm, CONTINUE) != NULL) { \
    if (fetch(vm, CONTINUE)->type != LABEL) { \
      assign(vm, VAL, make_error(vm, "invalid state")); \
      return; \
    } \
    void *label = object_data(fetch(vm, CONTINUE), void*); \
    goto *label; \
  } \
  return; \
} while(0);

object_t *make_label(vm_t *vm, void *label) {
  object_t *obj = make(vm, LABEL, sizeof(void*));
  object_data(obj, void*) = label;
  return obj;
}

object_t *reverse(vm_t *vm, object_t *args, object_t *acc) {
  if (args == NULL) return acc;
  return reverse(vm, cdr(vm, args), cons(vm, car(vm, args), acc));
}

void eval(vm_t *vm) {
tailcall:
  vm_gc(vm);

  if (fetch(vm, EXPR) == NULL) RET(NULL)
  if (fetch(vm, EXPR)->type == SYMBOL) RET(lookup(vm, fetch(vm, ENV), fetch(vm, EXPR)))
  if (fetch(vm, EXPR)->type != PAIR) RET(fetch(vm, EXPR))

  SAVE
  assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
  assign(vm, CONTINUE, make_label(vm, &&procedure_continue));
  goto tailcall;
procedure_continue:
  RESTORE
  assign(vm, FUN, fetch(vm, VAL));

  if (fetch(vm, FUN) == NULL) RET(make_error(vm, "nil is not operator"))

  assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));

  if (fetch(vm, FUN)->type == SPECIAL) {
    switch (object_data(fetch(vm, FUN), special_t)) {
      case F_IF:
        SAVE
        assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
        assign(vm, CONTINUE, make_label(vm, &&if_continue));
        goto tailcall;
if_continue:
        RESTORE
        if (true(error(fetch(vm, VAL)))) RET(fetch(vm, VAL))
        assign(vm, EXPR, !false(fetch(vm, VAL))
            ? car(vm, cdr(vm, fetch(vm, EXPR)))
            : car(vm, cdr(vm, cdr(vm, fetch(vm, EXPR)))));
        goto tailcall;

      case F_QUOTE:
        RET(car(vm, fetch(vm, EXPR)))

      case F_DEFINE:
        if (true(pair(car(vm, fetch(vm, EXPR))))) {
          SAVE
          assign(vm, EXPR, cons(vm, sym_lambda, cons(vm, cdr(vm, car(vm, fetch(vm, EXPR))), cdr(vm, fetch(vm, EXPR)))));
          assign(vm, CONTINUE, make_label(vm, &&define_continue_1));
          goto tailcall;
define_continue_1:
          RESTORE

          define(vm, fetch(vm, ENV), car(vm, car(vm, fetch(vm, EXPR))), fetch(vm, VAL));
        } else {
          SAVE
          assign(vm, EXPR, car(vm, cdr(vm, fetch(vm, EXPR))));
          assign(vm, CONTINUE, make_label(vm, &&define_continue_2));
          goto tailcall;
define_continue_2:
          RESTORE

          define(vm, fetch(vm, ENV), car(vm, fetch(vm, EXPR)), fetch(vm, VAL));
        }

        RET(&t)

      case F_LAMBDA:
        RET(make_procedure(vm,
            fetch(vm, ENV),
            car(vm, fetch(vm, EXPR)),
            cdr(vm, fetch(vm, EXPR))))

      case F_BEGIN:
        if (fetch(vm, EXPR) == NULL) RET(NULL)
begin_enter:
        if (cdr(vm, fetch(vm, EXPR)) == NULL) goto begin_exit;

        SAVE
        assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
        assign(vm, CONTINUE, make_label(vm, &&begin_continue));
        goto tailcall;
begin_continue:
        RESTORE

        assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));

        goto begin_enter;
begin_exit:
        assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
        goto tailcall;

      case F_AND:
        if (fetch(vm, EXPR) == NULL) RET(&t)
and_enter:
        if (cdr(vm, fetch(vm, EXPR)) == NULL) goto and_exit;

        SAVE
        assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
        assign(vm, CONTINUE, make_label(vm, &&and_continue));
        goto tailcall;
and_continue:
        RESTORE

        if (false(fetch(vm, VAL))) RET(fetch(vm, VAL))
        assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));

        goto and_enter;
and_exit:
        assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
        goto tailcall;

      case F_OR:
        if (fetch(vm, EXPR) == NULL) RET(&f)
or_enter:
        if (cdr(vm, fetch(vm, EXPR)) == NULL) goto or_exit;

        SAVE
        assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
        assign(vm, CONTINUE, make_label(vm, &&or_continue));
        goto tailcall;
or_continue:
        RESTORE

        if (!false(fetch(vm, VAL))) RET(fetch(vm, VAL))
        assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));

        goto or_enter;
or_exit:
        assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
        goto tailcall;

      case F_COND:
cond_enter:
        if (fetch(vm, EXPR) == NULL) goto cond_exit;

        if (true(object_eq(vm, car(vm, car(vm, fetch(vm, EXPR))), make_symbol(vm, "else")))) {
          assign(vm, EXPR, car(vm, cdr(vm, car(vm, fetch(vm, EXPR)))));
          goto tailcall;
        }

        SAVE
        assign(vm, EXPR, car(vm, car(vm, fetch(vm, EXPR))));
        assign(vm, CONTINUE, make_label(vm, &&cond_continue));
        goto tailcall;
cond_continue:
        RESTORE

        if (true(fetch(vm, VAL))) {
          assign(vm, EXPR, car(vm, cdr(vm, car(vm, fetch(vm, EXPR)))));
          goto tailcall;
        }

        assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));

        goto cond_enter;
cond_exit:
        goto tailcall;

      case F_EVAL:
        SAVE
        assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
        assign(vm, CONTINUE, make_label(vm, &&eval_continue));
        goto tailcall;
eval_continue:
        RESTORE
        assign(vm, EXPR, fetch(vm, VAL));
        goto tailcall;

      default: RET(make_error(vm, "oh no!!!"))
    }
  } else if (fetch(vm, FUN)->type == PRIMITIVE) {
    SAVE
    assign(vm, CONTINUE, make_label(vm, &&eval_primitive));
    goto eval_sequence;
eval_primitive:
    RESTORE
    RET((object_data(fetch(vm, FUN), primitive))(vm, fetch(vm, VAL)))
  } else if (fetch(vm, FUN)->type == PROCEDURE) {

    SAVE
    assign(vm, CONTINUE, make_label(vm, &&eval_procedure));
    goto eval_sequence;
eval_procedure:
    RESTORE

    object_t *vals = fetch(vm, VAL);

    object_t *body = cons(vm, sym_begin, object_data(fetch(vm, FUN), proc_t).body);
    object_t *parent = object_data(fetch(vm, FUN), proc_t).env; // captured environment
    object_t *params = object_data(fetch(vm, FUN), proc_t).params;

    object_t *vars = params;

    if (true(symbol(vars))) {
      vars = cons(vm, vars, NULL);
      vals = cons(vm, vals, NULL);
    }

    assign(vm, ENV, extend_frame(vm, vars, vals, parent));
    assign(vm, EXPR, body);

    goto tailcall;
  } else if (fetch(vm, FUN)->type == ERROR) {
    RET(fetch(vm, FUN))
  } else {
    RET(make_error(vm, "not a procedure"))
  }

eval_sequence:
  assign(vm, ARGL, NULL);

eval_sequence_enter:
  if (fetch(vm, EXPR) == NULL) goto eval_sequence_exit;
    SAVE
    assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
    assign(vm, CONTINUE, make_label(vm, &&eval_sequence_continue));
    goto tailcall;
eval_sequence_continue:
    RESTORE
    assign(vm, ARGL, cons(vm, fetch(vm, VAL), fetch(vm, ARGL)));
    assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));
  goto eval_sequence_enter;
eval_sequence_exit:

  RET(reverse(vm, fetch(vm, ARGL), NULL))
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

