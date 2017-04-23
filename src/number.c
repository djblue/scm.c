#include "number.h"

#include "error.h"
#include "pair.h"
#include "env.h"
#include "symbol.h"
#include "primitive.h"

#define FIXNUM_TAG 0x1

#define ASSERT_NUMBER(op,value)\
  if (!true(number(value))) { \
    return make_error(vm, op": operand is not a number", car(args)); \
  }

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

object_t number_eq(vm_t *vm, object_t a, object_t b) {
  if (a == NULL || b == NULL) return f;
  if (scm_type(a) != FIXNUM || scm_type(b) != FIXNUM) return f;
  return (scm_fixnum(a) == scm_fixnum(b)) ? t : f;
}

static object_t number(object_t o) {
  if (scm_type(o) != FIXNUM) {
    return f;
  }
  return t;
}

static object_t numberp(vm_t *vm, object_t args) {
  return number(car(args));
}

static object_t scm_add(vm_t *vm, object_t args) {
  long result = 0;

  while (args != NULL) {
    ASSERT_NUMBER("+", car(args))
    result += scm_fixnum(car(args));
    args = cdr(args);
  }

  return make_fixnum_int(vm, result);
}

static object_t scm_sub(vm_t *vm, object_t args) {
  ASSERT_NUMBER("-", car(args))

  long result = scm_fixnum(car(args));
  args = cdr(args);

  if (args == NULL) {
    result = 0 - result;
  }

  while (args != NULL) {
    ASSERT_NUMBER("-", car(args))
    result -= scm_fixnum(car(args));
    args = cdr(args);
  }

  return make_fixnum_int(vm, result);
}

static object_t scm_mul(vm_t *vm, object_t args) {
  long result = 1;

  while (args != NULL) {
    ASSERT_NUMBER("*", car(args))
    result *= scm_fixnum(car(args));
    args = cdr(args);
  }

  return make_fixnum_int(vm, result);
}

#define RELATIONAL(c_name,scm_name,op) \
  static object_t c_name(vm_t *vm, object_t args) { \
    ASSERT_NUMBER(scm_name, car(args)) \
    long prev = scm_fixnum(car(args)); \
    args = cdr(args); \
    while (args != NULL) { \
      ASSERT_NUMBER(scm_name, car(args)) \
      long operand = scm_fixnum(car(args)); \
      if (!(prev op operand)) return f; \
      prev = operand; \
      args = cdr(args); \
    } \
    return t; \
  }

RELATIONAL(scm_lt, "<", <)
RELATIONAL(scm_lte, "<=", <=)
RELATIONAL(scm_gt, ">", >)
RELATIONAL(scm_gte, ">=", >=)

void define_number(vm_t *vm, object_t env) {
  def("+", scm_add)
  def("-", scm_sub)
  def("*", scm_mul)
  def("<", scm_lt)
  def("<=", scm_lte)
  def(">", scm_gt)
  def(">=", scm_gte)
  def("number?", numberp)
}

