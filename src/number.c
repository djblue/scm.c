#include "number.h"
#include "error.h"
#include "pair.h"
#include "env.h"
#include "symbol.h"
#include "primitive.h"

#define FIXNUM_TAG 0x1

object_t make_fixnum(vm_t *vm, char *str) {
  long value = (atol(str) << 1) | FIXNUM_TAG;
  return (object_t) value;
}

static object_t make_fixnum_int(vm_t *vm, long fix) {
  return (object_t) ((fix << 1) | FIXNUM_TAG);
}

long scm_fixnum(object_t o) {
  return ((long) o) >> 1;
}

int scm_is_fixnum(object_t o) {
  return (((long) o) & 0x1) == FIXNUM_TAG;
}

static object_t number(object_t o) {
  if (scm_type(o) != FIXNUM) {
    return f;
  }
  return t;
}

static object_t plus(vm_t *vm, object_t a, object_t b) {
  if (a == NULL) return plus(vm, make_fixnum_int(vm, 0), b);
  if (b == NULL) return plus(vm, a, make_fixnum_int(vm, 0));

  if (!true(number(a)))
    return make_error(vm,
        "+: operand is not a number", a);

  if (!true(number(b)))
    return make_error(vm,
        "+: operand is not a number", b);

  return make_fixnum_int(vm, scm_fixnum(a) + scm_fixnum(b));
}

static object_t minus(vm_t *vm, object_t a, object_t b) {
  if (b == NULL) return minus(vm, make_fixnum_int(vm, 0), a);

  if (!true(number(a)))
    return make_error(vm,
        "-: operand is not a number", a);

  if (!true(number(b)))
    return make_error(vm,
        "-: operand is not a number", b);

  return make_fixnum_int(vm, scm_fixnum(a) - scm_fixnum(b));
}

static object_t multiply(vm_t *vm, object_t a, object_t b) {
  if (a == NULL) return multiply(vm, make_fixnum_int(vm, 1), b);
  if (b == NULL) return multiply(vm, a, make_fixnum_int(vm, 1));

  if (!true(number(a)))
    return make_error(vm,
        "*: operand is not a number", a);

  if (!true(number(b)))
    return make_error(vm,
        "*: operand is not a number", b);

  return make_fixnum_int(vm, scm_fixnum(a) * scm_fixnum(b));
}

object_t number_eq(vm_t *vm, object_t a, object_t b) {
  if (a == NULL || b == NULL) return f;
  if (scm_type(a) != FIXNUM || scm_type(b) != FIXNUM) return f;
  return (scm_fixnum(a) == scm_fixnum(b)) ? t : f;
}

static object_t numberp(vm_t *vm, object_t args) {
  object_t o = car(vm, args);
  if (true(error(o))) return o;
  return number(o);
}

static object_t eval_plus(vm_t *vm, object_t args) {
  if (args == NULL) return NULL;
  object_t op = car(vm, args);
  if (true(error(op))) return op;
  return plus(vm, op, eval_plus(vm, cdr(vm, args)));
}

static object_t eval_minus(vm_t *vm, object_t args) {
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

static object_t eval_multiply(vm_t *vm, object_t args) {
  if (args == NULL) return NULL;
  object_t op = car(vm, args);
  if (true(error(op))) return op;
  return multiply(vm, op, eval_multiply(vm, cdr(vm, args)));
}

void define_number(vm_t *vm, object_t env) {
  def("+", eval_plus)
  def("-", eval_minus)
  def("*", eval_multiply)
  def("number?", numberp)
}

