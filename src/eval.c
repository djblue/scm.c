#pragma GCC diagnostic ignored "-Wpedantic"

#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "eval.h"
#include "print.h"

static volatile int interrupt = 0;

void sigint_handler(int sig) {
  interrupt = 1;
}

#define SAVE if (!save(vm)) RET(stack_overflow)
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
  if (interrupt) RET(make_error(vm, "execution interrupted", NULL))

  vm_gc(vm);

  if (fetch(vm, EXPR) == NULL) RET(NULL)
  if (scm_type(fetch(vm, EXPR)) == SYMBOL) RET(lookup(vm, fetch(vm, ENV), fetch(vm, EXPR)))
  if (scm_type(fetch(vm, EXPR)) != PAIR) RET(fetch(vm, EXPR))

  RECUR(car(vm, fetch(vm, EXPR)), procedure_continue)
  assign(vm, FUN, fetch(vm, VAL));

  if (fetch(vm, FUN) == NULL) RET(make_error(vm, "Nil is not operator.", NULL))

  assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));

  switch(scm_type(fetch(vm, FUN))) {
    case SPECIAL:
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

      case F_SET:
        RECUR(car(vm, cdr(vm, fetch(vm, EXPR))), set_continue)
        RET(set(vm, fetch(vm, ENV), car(vm, fetch(vm, EXPR)), fetch(vm, VAL)))

      case F_LAMBDA:
        RET(make_procedure(vm,
            fetch(vm, ENV),
            car(vm, fetch(vm, EXPR)),
            cdr(vm, fetch(vm, EXPR)), 0))

      case F_MACRO:
        RET(make_procedure(vm,
            fetch(vm, ENV),
            car(vm, fetch(vm, EXPR)),
            cdr(vm, fetch(vm, EXPR)), 1))

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

      case F_APPLY:
        RECUR(car(vm, fetch(vm, EXPR)), apply_continue_1)
        assign(vm, FUN, fetch(vm, VAL));
        RECUR(car(vm, cdr(vm, fetch(vm, EXPR))), apply_continue_2)
        if (scm_type(fetch(vm, VAL)) != NIL && scm_type(fetch(vm, VAL)) != PAIR)
          RET(make_error(vm, "apply: cannot apply non-list args", fetch(vm, VAL)))
        assign(vm, ARGL, fetch(vm, VAL));
        goto apply;

      case F_MACROEXPAND:
        RECUR(car(vm, fetch(vm, EXPR)), macroexpand_continue_1)
        vm_set_macro_expand(vm, 1);
        RECUR(fetch(vm, VAL), macroexpand_continue_2)
        vm_set_macro_expand(vm, 0);
        RET(fetch(vm, VAL))

      default: RET(make_error(vm, "Unknown special operator.", fetch(vm, FUN)))
    }

    case PRIMITIVE:
    case PROCEDURE:
      assign(vm, ARGL, NULL);

      while (fetch(vm, EXPR) != NULL) {
        RECUR(car(vm, fetch(vm, EXPR)), eval_sequence_continue)
        assign(vm, ARGL, cons(vm, fetch(vm, VAL), fetch(vm, ARGL)));
        assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));
      }

      assign(vm, ARGL, reverse(vm, fetch(vm, ARGL), NULL));
    goto apply;

    case MACRO:
      SAVE
      assign(vm, ARGL, fetch(vm, EXPR));
      assign(vm, CONTINUE, &&macro_expand_continue);
      goto apply;
macro_expand_continue:
      RESTORE
      if (vm_macro_expand(vm)) {
        RET(fetch(vm, VAL))
      } else {
        assign(vm, EXPR, fetch(vm, VAL));
        goto tailcall;
      }

    default:
    RET(make_error(vm, "Not a procedure", fetch(vm, FUN)))
  }

apply:
  switch (scm_type(fetch(vm, FUN))) {
    case PRIMITIVE:
      RET((object_data(fetch(vm, FUN), primitive))(vm, fetch(vm, ARGL)))
    case MACRO:
    case PROCEDURE: {
      object_t body = object_data(fetch(vm, FUN), proc_t).body;
      object_t parent = object_data(fetch(vm, FUN), proc_t).env;
      object_t vars = object_data(fetch(vm, FUN), proc_t).params;
      object_t vals = fetch(vm, ARGL);

      assign(vm, ENV, extend_frame(vm, vars, vals, parent));
      assign(vm, EXPR, body);

      if (fetch(vm, EXPR) == NULL) RET(NULL)
      while (cdr(vm, fetch(vm, EXPR)) != NULL) {
        RECUR(car(vm, fetch(vm, EXPR)), apply_continue)
        assign(vm, EXPR, cdr(vm, fetch(vm, EXPR)));
      }
      assign(vm, EXPR, car(vm, fetch(vm, EXPR)));
      goto tailcall;
    }
    default:
    RET(make_error(vm, "apply: cannot apply", fetch(vm, FUN)))
  }

}

object_t scm_eval(vm_t *vm, object_t expr) {
  interrupt = 0;
  signal(SIGINT, sigint_handler);
  assign(vm, EXPR, expr);
  object_t env = fetch(vm, ENV);
  eval(vm);
  assign(vm, ENV, env);
  signal(SIGINT, SIG_DFL);
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
  stack_overflow = scm_guard(make_error(vm, "stack overflow", NULL));

  // special forms
  sym_if = defs("if", F_IF)
  sym_quote = defs("quote", F_QUOTE)
  sym_lambda = defs("lambda", F_LAMBDA)
  sym_macro = defs("macro", F_MACRO)
  sym_macroexpand = defs("macroexpand", F_MACROEXPAND)
  sym_begin = defs("begin", F_BEGIN)
  sym_and = defs("and", F_AND)
  sym_or = defs("or", F_OR)
  sym_cond = defs("cond", F_COND)
  sym_case = defs("case", F_CASE)
  sym_define = defs("define", F_DEFINE)
  sym_set = defs("set!", F_SET)
  sym_eval = defs("eval", F_EVAL)
  sym_apply = defs("apply", F_APPLY)

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

