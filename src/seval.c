#pragma GCC diagnostic ignored "-Wpedantic"

#include "vm.h"
#include "seval.h"
#include "eval.h"

#include "pair.h"
#include "env.h"
#include "symbol.h"
#include "primitive.h"
#include "print.h"

static void print_stack(vm_t *vm, object_t stack[], int n) {
  printf("-------- top --------\n");
  for (int i = 0; i < n; i++) {
    print(vm, stack[n - i - 1]);
  }
  printf("-------- bottom --------\n");
}

static object_t scm_seval(vm_t *vm, size_t n, object_t args[]) {

  if (n != 1) {
    return make_error(vm, "seval: incorrect argument count", NULL);
  }

  object_t a = NULL;        // the accumulator
  object_t x = args[0];     // the next expression
  long f = 0;               // the current frame pointer
  object_t c = NULL;        // the current closure
  long s = 0;               // the current stack pointer

  object_t ret =
    list(vm, 2, make_fixnum_int(vm, 18), make_fixnum_int(vm, 0));
  object_t next = cdr(ret);

  object_t stack[8192];

#define PUSH(value) stack[s++] = value;

#define INDEX(s,i) stack[(s - i) - 1]
#define INDEX_SET(s,i,v) stack[(s - i) - 1] = v

  void *label;
  static void *labels[] = {
    &&HALT,
    &&REFER_LOCAL,
    &&REFER_FREE,
    &&REFER_GLOBAL,
    &&INDIRECT,
    &&CONSTANT,
    &&CLOSE,
    &&BOX,
    &&TEST,
    &&ASSIGN_LOCAL,
    &&ASSIGN_FREE,
    &&ASSIGN_GLOBAL,
    &&CONTI,
    &&NUATE,
    &&FRAME,
    &&ARGUMENT,
    &&SHIFT,
    &&APPLY,
    &&RETURN
  };

#define CONTINUE label = labels[scm_fixnum(car(x))]; goto *label;

  CONTINUE

HALT:
  return a;

REFER_LOCAL:
  a = INDEX(f, scm_fixnum(cadr(x)));
  x = caddr(x);
  CONTINUE

REFER_FREE:
  a = scm_vector_ref(c, scm_fixnum(cadr(x)) + 1);
  x = caddr(x);
  CONTINUE

REFER_GLOBAL:
  a = symbol_get_binding(cadr(x));
  x = caddr(x);
  CONTINUE

INDIRECT:
  CONTINUE

CONSTANT:
  a = cadr(x);
  x = caddr(x);
  CONTINUE

CLOSE: {
  long n = scm_fixnum(cadr(x));
  object_t body = caddr(x);

  object_t v = scm_make_vector(vm, n + 1);
  scm_vector_set(v, 0, body);

  for (int i = 0; i < n; i++) {
    scm_vector_set(v, i + 1, INDEX(s, i));
  }

  a = v;
  s = s - n;
  x = car(cdddr(x));

  CONTINUE
}

BOX:
  CONTINUE

TEST:
  x = (a == t) ? cadr(x) : caddr(x);
  CONTINUE

ASSIGN_LOCAL:
  CONTINUE

ASSIGN_FREE:
  CONTINUE

ASSIGN_GLOBAL: {
  object_t sym = cadr(x);
  define(vm, fetch(vm, ENV), sym, a);
  x = caddr(x);
  CONTINUE
}

CONTI:
  CONTINUE

NUATE:
  CONTINUE

FRAME: {
  object_t ret = cadr(x);
  PUSH(c)
  PUSH(make_fixnum_int(vm, f))
  PUSH(ret)
  x = caddr(x);
  CONTINUE
}

ARGUMENT:
  PUSH(a)
  x = cadr(x);
  CONTINUE

SHIFT: {
  long n = scm_fixnum(cadr(x));
  long m = scm_fixnum(caddr(x));

  for (int i = n - 1; i >= 0; i--) {
    stack[s - (i + m) - 1] = stack[s - i - 1];
  }

  s = s - m;
  x = car(cdddr(x));
  CONTINUE
}

APPLY: {
  long n = scm_fixnum(cadr(x));

  if (scm_type(a) == VECTOR) {
    x = scm_vector_ref(a, 0);
    f = s;
    c = a;
  } else if (scm_type(a) == PRIMITIVE) {
    a = (object_data(a, primitive))(vm, n, &stack[s - n]);
    set_car(next, make_fixnum_int(vm, n));
    x = ret;
  } else if (scm_type(a) == PROCEDURE) {
    object_t args = NULL;
    for (int i = n; i > 0; i--) {
      args = cons(vm, stack[s - i], args);
    }
    args = list(vm, 2, sym_quote, args);
    object_t form = list(vm, 3, sym_apply, a, args);
    a = scm_eval(vm, form);
    x = list(vm, 2, make_fixnum_int(vm, 17), make_fixnum_int(vm, n));
  }

  CONTINUE
}

RETURN: {
  long n = scm_fixnum(cadr(x));
  s = s - n;
  x = INDEX(s,0);
  f = scm_fixnum(INDEX(s,1));
  c = INDEX(s,2);
  s = s - 3;
  CONTINUE
}

  return NULL;
}

void define_seval(vm_t *vm, object_t env) {
  def("seval", scm_seval)
}

