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

object_t number_eq(vm_t *vm, object_t a, object_t b) {
  if (a == NULL || b == NULL) return f;
  if (scm_type(a) != FIXNUM || scm_type(b) != FIXNUM) return f;
  return (scm_fixnum(a) == scm_fixnum(b)) ? t : f;
}

static object_t numberp(vm_t *vm, object_t args) {
  object_t o = car(args);
  if (true(error(o))) return o;
  return number(o);
}

static object_t scm_add(vm_t *vm, object_t args) {
  long result = 0;

  while (args != NULL) {
    if (!true(number(car(args)))) {
      return make_error(vm, "+: operand is not a number", car(args));
    }

    result += scm_fixnum(car(args));
    args = cdr(args);
  }

  return make_fixnum_int(vm, result);
}

static object_t scm_sub(vm_t *vm, object_t args) {
  if (!true(number(car(args)))) {
    return make_error(vm, "-: operand is not a number", car(args));
  }

  long result = scm_fixnum(car(args));
  args = cdr(args);

  if (args == NULL) {
    result = 0 - result;
  }

  while (args != NULL) {
    if (!true(number(car(args)))) {
      return make_error(vm, "-: operand is not a number", car(args));
    }

    result -= scm_fixnum(car(args));
    args = cdr(args);
  }

  return make_fixnum_int(vm, result);
}

static object_t scm_mul(vm_t *vm, object_t args) {
  long result = 1;

  while (args != NULL) {
    if (!true(number(car(args)))) {
      return make_error(vm, "*: operand is not a number", car(args));
    }

    result *= scm_fixnum(car(args));
    args = cdr(args);
  }

  return make_fixnum_int(vm, result);
}

static object_t scm_lt(vm_t *vm, object_t args) {
  if (!true(number(car(args)))) {
    return make_error(vm, "<: operand is not a number", car(args));
  }

  long prev = scm_fixnum(car(args));
  args = cdr(args);

  while (args != NULL) {
    if (!true(number(car(args)))) {
      return make_error(vm, "<: operand is not a number", car(args));
    }

    long operand = scm_fixnum(car(args));
    if (!(prev < operand)) {
      return f;
    }

    prev = operand;
    args = cdr(args);
  }

  return t;
}

void define_number(vm_t *vm, object_t env) {
  def("+", scm_add)
  def("-", scm_sub)
  def("*", scm_mul)
  def("<", scm_lt)
  def("number?", numberp)
}

