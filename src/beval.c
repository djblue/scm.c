#pragma GCC diagnostic ignored "-Wpedantic"

#include "beval.h"

#include "pair.h"
#include "env.h"
#include "symbol.h"
#include "primitive.h"

/*typedef enum {
  HALT = 0,
  REFER,
  CONSTANT,
  CLOSE,
  TEST,
  ASSIGN,
  CONTI,
  NUATE,
  FRAME,
  ARGUMENT,
  APPLY,
  RETURN
} op_t;*/

static object_t scm_lookup(object_t access, object_t e) {
  long rib, elt;

  for (rib = scm_fixnum(car(access)); rib > 0; rib--) {
    e = cdr(e);
  }

  object_t r = car(e);

  for (elt = scm_fixnum(cdr(access)); elt > 0; elt--) {
    r = cdr(r);
  }
  
  return r;
}

static object_t scm_closure(vm_t *vm, object_t body, object_t e) {
  return list(vm, 2, body, e);
}

static object_t scm_continuation(vm_t *vm, object_t s) {
  object_t zero = make_fixnum_int(vm, 0);
  object_t body = list(vm, 3, make_fixnum_int(vm, 6), s, cons(vm, zero, zero));
  return scm_closure(vm, body, NULL);
}

static object_t scm_call_frame(vm_t *vm, object_t x, object_t e, object_t r, object_t s) {
  return list(vm, 4, x, e, r, s);
}

static object_t scm_extend(vm_t *vm, object_t r, object_t e) {
  return cons(vm, e, r);
}

static object_t scm_beval(vm_t *vm, object_t args) {

  object_t a = NULL;        // the accumulator
  object_t x = car(args);   // the next expression
  object_t e = cadr(args);  // the current environment
  object_t r = NULL;        // the current value rib
  object_t s = NULL;        // the current stack

  int sp = 0;
  object_t stack[8192];

#define PUSH(value) stack[sp++] = value;
#define POP stack[--sp]

  object_t ret = list(vm, 1, make_fixnum_int(vm, 11));

  void *label;
  static void *labels[] = {
    &&HALT,
    &&REFER,
    &&CONSTANT,
    &&CLOSE,
    &&TEST,
    &&ASSIGN,
    &&CONTI,
    &&NUATE,
    &&FRAME,
    &&ARGUMENT,
    &&APPLY,
    &&RETURN
  };


#define CONTINUE label = labels[scm_fixnum(car(x))]; goto *label;

  CONTINUE

HALT: 
  return a;
REFER:
  a = car(scm_lookup(cadr(x), e));
  x = caddr(x);
  CONTINUE
CONSTANT:
  a = cadr(x);
  x = caddr(x);
  CONTINUE
CLOSE:
  a = scm_closure(vm, cadr(x), e);
  x = caddr(x);
  CONTINUE
TEST:
  x = (a == t) ? cadr(x) : caddr(x);
  CONTINUE
ASSIGN:
  set_car(scm_lookup(cadr(x), e), a);
  x = caddr(x);
  CONTINUE
CONTI:
  a = scm_continuation(vm, s);
  x = cadr(x);
  CONTINUE
NUATE:
  a = car(scm_lookup(caddr(x), e));
  s = cadr(x);
  x = ret;
  CONTINUE
FRAME:
  //s = scm_call_frame(vm, cadr(x), e, r, s);
  PUSH(cadr(x))
  PUSH(e)
  PUSH(r)
  PUSH(s)
  r = NULL;
  x = caddr(x);
  CONTINUE
ARGUMENT:
  r = cons(vm, a, r);
  x = cadr(x);
  CONTINUE
APPLY:
  if (scm_type(a) == PRIMITIVE) {
    a = object_data(a, primitive)(vm, r);
    x = ret;
  } else {
    x = car(a);
    e = scm_extend(vm, cadr(a), r);
    r = NULL;
  }
  CONTINUE
RETURN:
  s = POP;
  r = POP;
  e = POP;
  x = POP;
  /*
  x = car(s);
  e = cadr(s);
  r = caddr(s);
  s = car(cdddr(s));
  */
  CONTINUE

  return NULL;
}

void define_beval(vm_t *vm, object_t env) {
  def("beval", scm_beval)
}

