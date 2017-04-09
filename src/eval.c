#pragma GCC diagnostic ignored "-Wpedantic"

#include <stdio.h>
#include <string.h>

#include "eval.h"
#include "print.h"

#define SAVE save(vm);
#define RESTORE restore(vm);

#define RET(value)do{\
  assign(vm, VAL, (value)); \
  if (scm_type(fetch(vm, VAL)) == ERROR) { \
    vm_reset(vm); \
    return; \
  } \
  if (fetch(vm, CONTINUE) != NULL) { \
    void *label = (void*) fetch(vm, CONTINUE); \
    goto *label; \
  } \
  return; \
} while(0);

#define RECUR(value,label)do{\
  SAVE \
  assign(vm, EXPR, value); \
  assign(vm, CONTINUE, &&label); \
  goto tailcall; \
label: \
  RESTORE \
} while(0);

object_t reverse(vm_t *vm, object_t args, object_t acc) {
  if (args == NULL) return acc;
  return reverse(vm, cdr(vm, args), cons(vm, car(vm, args), acc));
}

static void eval(vm_t *vm) {
tailcall:
  vm_gc(vm);

  if (fetch(vm, EXPR) == NULL) RET(NULL)
  if (scm_type(fetch(vm, EXPR)) == SYMBOL) RET(lookup(vm, fetch(vm, ENV), fetch(vm, EXPR)))
  if (scm_type(fetch(vm, EXPR)) != PAIR) RET(fetch(vm, EXPR))

  RECUR(car(vm, fetch(vm, EXPR)), procedure_continue)
  assign(vm, FUN, fetch(vm, VAL));

  if (fetch(vm, FUN) == NULL) RET(make_error(vm, "Nil is not operator.", NULL))

  assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));

  if (scm_type(fetch(vm, FUN)) == SPECIAL) {
    switch (object_data(fetch(vm, FUN), special_t)) {
      case F_IF:
        RECUR(car(vm, fetch(vm, EXPR)), if_continue)
        if (true(error(fetch(vm, VAL)))) RET(fetch(vm, VAL))
        assign(vm, EXPR, !false(fetch(vm, VAL))
            ? car(vm, cdr(vm, fetch(vm, EXPR)))
            : car(vm, cdr(vm, cdr(vm, fetch(vm, EXPR)))));
        goto tailcall;

      case F_QUOTE:
        RET(car(vm, fetch(vm, EXPR)))

      case F_DEFINE:
        if (true(pair(car(vm, fetch(vm, EXPR))))) {
          RECUR(cons(vm, sym_lambda, cons(vm, cdr(vm, car(vm, fetch(vm, EXPR))), cdr(vm, fetch(vm, EXPR)))), define_continue_1)
          define(vm, fetch(vm, ENV), car(vm, car(vm, fetch(vm, EXPR))), fetch(vm, VAL));
        } else {
          RECUR(car(vm, cdr(vm, fetch(vm, EXPR))), define_continue_2)
          define(vm, fetch(vm, ENV), car(vm, fetch(vm, EXPR)), fetch(vm, VAL));
        }

        RET(t)

      case F_LAMBDA:
        RET(make_procedure(vm,
            fetch(vm, ENV),
            car(vm, fetch(vm, EXPR)),
            cons(vm, sym_begin, cdr(vm, fetch(vm, EXPR)))))

      case F_BEGIN:
        if (fetch(vm, EXPR) == NULL) RET(NULL)
        while (cdr(vm, fetch(vm, EXPR)) != NULL) {
          RECUR(car(vm, fetch(vm, EXPR)), begin_continue)
          assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));
        }
        assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
        goto tailcall;

      case F_AND:
        if (fetch(vm, EXPR) == NULL) RET(t)
        while (cdr(vm, fetch(vm, EXPR)) != NULL) {
          RECUR(car(vm, fetch(vm, EXPR)), and_continue)
          if (false(fetch(vm, VAL))) RET(fetch(vm, VAL))
          assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));
        }
        assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
        goto tailcall;

      case F_OR:
        if (fetch(vm, EXPR) == NULL) RET(f)
        while (cdr(vm, fetch(vm, EXPR)) != NULL) {
          RECUR(car(vm, fetch(vm, EXPR)), or_continue)
          if (!false(fetch(vm, VAL))) RET(fetch(vm, VAL))
          assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));
        }
        assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
        goto tailcall;

      case F_COND:
        while (fetch(vm, EXPR) != NULL) {
          if (true(object_eq(vm, car(vm, car(vm, fetch(vm, EXPR))), sym_else))) {
            assign(vm, EXPR, car(vm, cdr(vm, car(vm, fetch(vm, EXPR)))));
            goto tailcall;
          }
          RECUR(car(vm, car(vm, fetch(vm, EXPR))), cond_continue)
          if (true(fetch(vm, VAL))) {
            assign(vm, EXPR, car(vm, cdr(vm, car(vm, fetch(vm, EXPR)))));
            goto tailcall;
          }

          assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));
        }
        goto tailcall;

      case F_CASE:
        RECUR(car(vm, fetch(vm, EXPR)), eval_case)
        assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));
        while (fetch(vm, EXPR) != NULL) {
          object_t entry = car(vm, fetch(vm, EXPR));
          if (true(object_eq(vm, car(vm, entry), sym_else))) {
              assign(vm, EXPR, car(vm, cdr(vm, entry)));
              goto tailcall;
          }
          object_t list = car(vm, entry);
          while (list != NULL) {
            if (true(object_eq(vm, car(vm, list), fetch(vm, VAL)))) {
              assign(vm, EXPR, car(vm, cdr(vm, entry)));
              goto tailcall;
            }
            list = cdr(vm, list);
          }
          assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));
        }
        goto tailcall;

      case F_EVAL:
        RECUR(car(vm, fetch(vm, EXPR)), eval_continue)
        assign(vm, EXPR, fetch(vm, VAL));
        goto tailcall;

      default: RET(make_error(vm, "Unknown special operator.", fetch(vm, FUN)))
    }
  } else if (scm_type(fetch(vm, FUN)) == PRIMITIVE) {
    SAVE
    assign(vm, CONTINUE, &&eval_primitive);
    goto eval_sequence;
eval_primitive:
    RESTORE
    RET((object_data(fetch(vm, FUN), primitive))(vm, fetch(vm, VAL)))
  } else if (scm_type(fetch(vm, FUN)) == PROCEDURE) {

    SAVE
    assign(vm, CONTINUE, &&eval_procedure);
    goto eval_sequence;
eval_procedure:
    RESTORE

    object_t vals = fetch(vm, VAL);

    object_t body = object_data(fetch(vm, FUN), proc_t).body;
    object_t parent = object_data(fetch(vm, FUN), proc_t).env; // captured environment
    object_t params = object_data(fetch(vm, FUN), proc_t).params;

    object_t vars = params;

    if (true(symbol(vars))) {
      vars = cons(vm, vars, NULL);
      vals = cons(vm, vals, NULL);
    }

    assign(vm, ENV, extend_frame(vm, vars, vals, parent));
    assign(vm, EXPR, body);

    goto tailcall;
  } else if (scm_type(fetch(vm, FUN)) == ERROR) {
    RET(fetch(vm, FUN))
  } else {
    RET(make_error(vm, "Not a procedure", fetch(vm, FUN)))
  }

eval_sequence:
  assign(vm, ARGL, NULL);

  while (fetch(vm, EXPR) != NULL) {
    RECUR(car(vm, fetch(vm, EXPR)), eval_sequence_continue)
    assign(vm, ARGL, cons(vm, fetch(vm, VAL), fetch(vm, ARGL)));
    assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));
  }

  RET(reverse(vm, fetch(vm, ARGL), NULL))
}

object_t scm_eval(vm_t *vm, object_t expr) {
  assign(vm, EXPR, expr);
  object_t env = fetch(vm, ENV);
  eval(vm);
  assign(vm, ENV, env);
  return fetch(vm, VAL);
}

#define eval_predicate(fn,p) \
  object_t fn(vm_t *vm, object_t args) { \
    object_t o = car(vm, args); \
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

object_t eval_cons(vm_t *vm, object_t args) {
  object_t a = car(vm, args);
  object_t b = car(vm, cdr(vm, args));
  return cons(vm, a, b);
}

object_t eval_car(vm_t *vm, object_t args) {
  object_t pair = car(vm, args);
  if (pair == NULL) return NULL;
  if (scm_type(pair) == ERROR) return pair;
  if (scm_type(pair) != PAIR) {
    return make_error(vm, "Object not pair.", pair);
  }
  return car(vm, pair);
}

object_t eval_cdr(vm_t *vm, object_t args) {
  object_t pair = car(vm, args);
  if (pair == NULL) return NULL;
  if (scm_type(pair) == ERROR) return pair;
  if (scm_type(pair) != PAIR) {
    return make_error(vm, "Object not pair.", pair);
  }
  return cdr(vm, pair);
}

object_t eval_set_car(vm_t *vm, object_t args) {
  object_t pair = car(vm, args);
  if (pair == NULL) return NULL;
  object_t value = car(vm, cdr(vm, args));
  if (scm_type(pair) == ERROR) return pair;
  if (scm_type(pair) != PAIR) {
    return make_error(vm, "Object not pair.", pair);
  }
  return set_car(vm, pair, value);
}

object_t eval_set_cdr(vm_t *vm, object_t args) {
  object_t pair = car(vm, args);
  if (pair == NULL) return NULL;
  object_t value = car(vm, cdr(vm, args));
  if (scm_type(pair) == ERROR) return pair;
  if (scm_type(pair) != PAIR) {
    return make_error(vm, "Object not pair.", pair);
  }
  return set_cdr(vm, pair, value);
}

object_t eval_print(vm_t *vm, object_t args) {
  object_t o = car(vm, args);
  print(vm, o);
  return o;
}

object_t eval_eq(vm_t *vm, object_t args) {
  if (args == NULL) return NULL;

  object_t a = car(vm, args);
  object_t ls = cdr(vm, args);

  while (ls != NULL) {
    object_t b = car(vm, ls);
    if (false(object_eq(vm, a, b))) return f;
    ls = cdr(vm, ls);
  }

  return t;
}

object_t procedurep(vm_t *vm, object_t args) {
  object_t proc = car(vm, args);
  switch (scm_type(proc)) {
    case SPECIAL:
    case PRIMITIVE:
    case PROCEDURE:
      return t;
    default:
      return f;
  }
}

#include "number.h"

object_t eval_plus(vm_t *vm, object_t args) {
  if (args == NULL) return NULL;
  object_t op = car(vm, args);
  if (true(error(op))) return op;
  return plus(vm, op, eval_plus(vm, cdr(vm, args)));
}

object_t eval_minus(vm_t *vm, object_t args) {
  if (args == NULL) return NULL;
  object_t op = car(vm, args);
  if (cdr(vm, args) == NULL) {
    op = minus(vm, op, NULL);
  }
  args = cdr(vm, args);
  while (args != NULL) {
    op = minus(vm, op, car(vm, args));
    args = cdr(vm, args);
  }
  return op;
}

object_t eval_multiply(vm_t *vm, object_t args) {
  if (args == NULL) return NULL;
  object_t op = car(vm, args);
  if (true(error(op))) return op;
  return multiply(vm, op, eval_multiply(vm, cdr(vm, args)));
}

object_t eq(vm_t *vm, object_t args) {
  if (args == NULL) return NULL;
  if (cdr(vm, args) == NULL) return NULL;
  if (car(vm, args) == car(vm, cdr(vm, args))) return t;
  return f;
}

object_t eval_error(vm_t *vm, object_t args) {
  object_t message = car(vm, args);
  if (scm_type(message) != STRING)
    return make_error(vm, "Cannot make error.", message);
  object_t irritant = car(vm, cdr(vm, args));
  return make_error(vm, string_cstr(message), irritant);
}

object_t interaction_environment(vm_t *vm, object_t args) {
  return fetch(vm, ENV);
}

void init(vm_t *vm, object_t env) {

  eof = scm_guard(make(vm, ENDOFINPUT, 0));
  ueof = scm_guard(make(vm, UENDOFINPUT, 0));
  t = scm_guard(make(vm, TRUE, 0));
  f = scm_guard(make(vm, FALSE, 0));
  sym_else = make_symbol(vm, "else");

  // special forms
  sym_if = defs("if", F_IF)
  sym_quote = defs("quote", F_QUOTE)
  sym_lambda = defs("lambda", F_LAMBDA)
  sym_begin = defs("begin", F_BEGIN)
  sym_and = defs("and", F_AND)
  sym_or = defs("or", F_OR)
  sym_cond = defs("cond", F_COND)
  sym_case = defs("case", F_CASE)
  sym_define = defs("define", F_DEFINE)
  sym_eval = defs("eval", F_EVAL)

  def("+", eval_plus)
  def("-", eval_minus)
  def("*", eval_multiply)
  def("=", eval_eq)

  def("interaction-environment", interaction_environment)

  def("number?", numberp)
  def("boolean?", booleanp)
  def("error?", errorp)
  def("string?", stringp)
  def("char?", characterp)
  def("symbol?", symbolp)
  def("pair?", pairp)
  def("null?", nullp)
  def("procedure?", procedurep)
  def("eq?", eq)

  def("error", eval_error)

  def("cons", eval_cons)
  def("car", eval_car)
  def("cdr", eval_cdr)
  def("set-car!", eval_set_car)
  def("set-cdr!", eval_set_cdr)

  def("write", eval_print)
}

