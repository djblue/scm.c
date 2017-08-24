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
  return reverse(vm, cdr(args), cons(vm, car(args), acc));
}

static void eval(vm_t *vm) {
  object_t args[256];

tailcall:
  if (interrupt) RET(make_error(vm, "eval: execution interrupted", NULL))

  vm_gc(vm);

  if (fetch(vm, EXPR) == NULL) RET(NULL)
  if (scm_type(fetch(vm, EXPR)) == SYMBOL) RET(lookup(vm, fetch(vm, ENV), fetch(vm, EXPR)))
  if (scm_type(fetch(vm, EXPR)) != PAIR) RET(fetch(vm, EXPR))

  RECUR(car(fetch(vm, EXPR)), procedure_continue)
  assign(vm, FUN, fetch(vm, VAL));

  if (fetch(vm, FUN) == NULL) RET(make_error(vm, "eval: nil is not operator", NULL))

  assign(vm, EXPR, cdr(fetch(vm, EXPR)));

  switch(scm_type(fetch(vm, FUN))) {
    case SPECIAL:
    switch (object_data(fetch(vm, FUN), special_t)) {
      case F_IF:
        RECUR(car(fetch(vm, EXPR)), if_continue)
        if (true(error(fetch(vm, VAL)))) RET(fetch(vm, VAL))
        assign(vm, EXPR, !false(fetch(vm, VAL))
            ? cadr(fetch(vm, EXPR)) : caddr(fetch(vm, EXPR)));
        goto tailcall;

      case F_QUOTE:
        RET(car(fetch(vm, EXPR)))

      case F_DEFINE:
        if (true(pair(car(fetch(vm, EXPR))))) {
          RECUR(cons(vm, sym_lambda, cons(vm, cdar(fetch(vm, EXPR)), cdr(fetch(vm, EXPR)))), define_continue_1)
          define(vm, fetch(vm, ENV), caar(fetch(vm, EXPR)), fetch(vm, VAL));
        } else {
          RECUR(cadr(fetch(vm, EXPR)), define_continue_2)
          define(vm, fetch(vm, ENV), car(fetch(vm, EXPR)), fetch(vm, VAL));
        }

        RET(t)

      case F_SET:
        RECUR(cadr(fetch(vm, EXPR)), set_continue)
        RET(set(vm, fetch(vm, ENV), car(fetch(vm, EXPR)), fetch(vm, VAL)))

      case F_LAMBDA:
        RET(make_procedure(vm,
            fetch(vm, ENV),
            car(fetch(vm, EXPR)),
            cdr(fetch(vm, EXPR)), 0))

      case F_MACRO:
        RET(make_procedure(vm,
            fetch(vm, ENV),
            car(fetch(vm, EXPR)),
            cdr(fetch(vm, EXPR)), 1))

      case F_BEGIN:
        if (fetch(vm, EXPR) == NULL) RET(NULL)
        while (cdr(fetch(vm, EXPR)) != NULL) {
          RECUR(car(fetch(vm, EXPR)), begin_continue)
          assign(vm, EXPR, cdr(fetch(vm, EXPR)));
        }
        assign(vm, EXPR, car(fetch(vm, EXPR)));
        goto tailcall;

      case F_AND:
        if (fetch(vm, EXPR) == NULL) RET(t)
        while (cdr(fetch(vm, EXPR)) != NULL) {
          RECUR(car(fetch(vm, EXPR)), and_continue)
          if (false(fetch(vm, VAL))) RET(fetch(vm, VAL))
          assign(vm, EXPR, cdr(fetch(vm, EXPR)));
        }
        assign(vm, EXPR, car(fetch(vm, EXPR)));
        goto tailcall;

      case F_OR:
        if (fetch(vm, EXPR) == NULL) RET(f)
        while (cdr(fetch(vm, EXPR)) != NULL) {
          RECUR(car(fetch(vm, EXPR)), or_continue)
          if (!false(fetch(vm, VAL))) RET(fetch(vm, VAL))
          assign(vm, EXPR, cdr(fetch(vm, EXPR)));
        }
        assign(vm, EXPR, car(fetch(vm, EXPR)));
        goto tailcall;

      case F_COND:
        while (fetch(vm, EXPR) != NULL) {
          if (true(object_eq(caar(fetch(vm, EXPR)), sym_else))) {
            assign(vm, EXPR, cadar(fetch(vm, EXPR)));
            goto tailcall;
          }
          RECUR(car(car(fetch(vm, EXPR))), cond_continue)
          if (true(fetch(vm, VAL))) {
            assign(vm, EXPR, cadar(fetch(vm, EXPR)));
            goto tailcall;
          }

          assign(vm, EXPR, cdr(fetch(vm, EXPR)));
        }
        goto tailcall;

      case F_CASE:
        RECUR(car(fetch(vm, EXPR)), eval_case)
        assign(vm, EXPR, cdr(fetch(vm, EXPR)));
        while (fetch(vm, EXPR) != NULL) {
          object_t entry = car(fetch(vm, EXPR));
          if (true(object_eq(car(entry), sym_else))) {
              assign(vm, EXPR, car(cdr(entry)));
              goto tailcall;
          }
          object_t list = car(entry);
          while (list != NULL) {
            if (true(object_eq(car(list), fetch(vm, VAL)))) {
              assign(vm, EXPR, car(cdr(entry)));
              goto tailcall;
            }
            list = cdr(list);
          }
          assign(vm, EXPR, cdr(fetch(vm, EXPR)));
        }
        goto tailcall;

      case F_EVAL:
        RECUR(car(fetch(vm, EXPR)), eval_continue)
        assign(vm, EXPR, fetch(vm, VAL));
        goto tailcall;

      case F_APPLY:
        RECUR(car(fetch(vm, EXPR)), apply_continue_1)
        assign(vm, FUN, fetch(vm, VAL));
        RECUR(cadr(fetch(vm, EXPR)), apply_continue_2)
        if (scm_type(fetch(vm, VAL)) != NIL && scm_type(fetch(vm, VAL)) != PAIR)
          RET(make_error(vm, "apply: cannot apply non-list args", fetch(vm, VAL)))
        assign(vm, ARGL, fetch(vm, VAL));
        goto apply;

      default: RET(make_error(vm, "eval: unknown special operator", fetch(vm, FUN)))
    }

    case PRIMITIVE:
    case PROCEDURE:
      assign(vm, ARGL, NULL);

      while (fetch(vm, EXPR) != NULL) {
        RECUR(car(fetch(vm, EXPR)), eval_sequence_continue)
        assign(vm, ARGL, cons(vm, fetch(vm, VAL), fetch(vm, ARGL)));
        assign(vm, EXPR, cdr(fetch(vm, EXPR)));
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
    RET(make_error(vm, "eval: not a procedure", fetch(vm, FUN)))
  }

apply:
  switch (scm_type(fetch(vm, FUN))) {
    case PRIMITIVE: {
      size_t n = 0;
      object_t argl = fetch(vm, ARGL);
      while (argl != NULL) {
        args[n++] = car(argl);
        argl = cdr(argl);
      }
      RET((object_data(fetch(vm, FUN), primitive))(vm, n, args))
    }
    case MACRO:
    case PROCEDURE: {
      object_t body = object_data(fetch(vm, FUN), proc_t).body;
      object_t parent = object_data(fetch(vm, FUN), proc_t).env;
      object_t vars = object_data(fetch(vm, FUN), proc_t).params;
      object_t vals = fetch(vm, ARGL);

      assign(vm, ENV, extend_frame(vm, vars, vals, parent));
      assign(vm, EXPR, body);

      if (fetch(vm, EXPR) == NULL) RET(NULL)
      while (cdr(fetch(vm, EXPR)) != NULL) {
        RECUR(car(fetch(vm, EXPR)), apply_continue)
        assign(vm, EXPR, cdr(fetch(vm, EXPR)));
      }
      assign(vm, EXPR, car(fetch(vm, EXPR)));
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

object_t symbolp(vm_t *vm, size_t n, object_t args[]) {
  if (n != 1) {
    return make_error(vm, "symbol?: incorrect argument count", NULL);
  }

  return symbol(args[0]);
}

object_t eval_eq(vm_t *vm, size_t n, object_t args[]) {
  if (n < 1) {
    return make_error(vm, "=: incorrect argument count", NULL);
  }

  object_t a = args[0];

  for (int i = 1; i < n; i++) {
    object_t b = args[i];
    if (object_eq(a, b) != t) return f;
  }

  return t;
}

object_t procedurep(vm_t *vm, size_t n, object_t args[]) {
  if (n != 1) {
    return make_error(vm, "procedure?: incorrect argument count", NULL);
  }

  switch (scm_type(args[0])) {
    case SPECIAL:
    case PRIMITIVE:
    case PROCEDURE:
      return t;
    default:
      return f;
  }
}

object_t macrop(vm_t *vm, size_t n, object_t args[]) {
  if (n != 1) {
    return make_error(vm, "procedure?: incorrect argument count", NULL);
  }

  return scm_type(args[0]) == MACRO ? t : f;
}

object_t eq(vm_t *vm, size_t n, object_t args[]) {
  if (n != 2) {
    return make_error(vm, "eq?: incorrect argument count", NULL);
  }

  return args[0] == args[1] ? t : f;
}

object_t interaction_environment(vm_t *vm, size_t n, object_t args[]) {
  if (n != 0) {
    return make_error(vm,
        "interaction-environment: incorrect argument count", NULL);
  }

  return fetch(vm, ENV);
}

void define_eval(vm_t *vm, object_t env) {
  sym_else = make_symbol(vm, "else");
  stack_overflow = scm_guard(make_error(vm, "stack overflow", NULL));

  // special forms
  sym_if = defs("if", F_IF)
  sym_quote = defs("quote", F_QUOTE)
  sym_lambda = defs("lambda", F_LAMBDA)
  sym_macro = defs("macro", F_MACRO)
  sym_begin = defs("begin", F_BEGIN)
  sym_and = defs("and", F_AND)
  sym_or = defs("or", F_OR)
  sym_cond = defs("cond", F_COND)
  sym_case = defs("case", F_CASE)
  sym_define = defs("define", F_DEFINE)
  sym_set = defs("set!", F_SET)
  sym_eval = defs("eval", F_EVAL)
  sym_apply = defs("apply", F_APPLY)

  def("=", eval_eq)

  def("interaction-environment", interaction_environment)

  def("symbol?", symbolp)
  def("procedure?", procedurep)
  def("macro?", macrop)
  def("eq?", eq)
}

