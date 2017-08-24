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

object_t make_fixnum_int(vm_t *vm, long fix) {
  return (object_t) ((fix << 1) | FIXNUM_TAG);
}

long scm_fixnum(object_t o) {
  return ((long) o) >> 1;
}

int scm_is_fixnum(object_t o) {
  return (((long) o) & 0x1) == FIXNUM_TAG;
}

object_t number_eq(object_t a, object_t b) {
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

static object_t numberp(vm_t *vm, size_t n, object_t args[]) {
  return number(args[0]);
}

static object_t scm_add(vm_t *vm, size_t n, object_t args[]) {
  long result = 0;

  for (int i = 0; i < n; i++) {
    ASSERT_NUMBER("+", args[i])
    result += scm_fixnum(args[i]);
  }

  return make_fixnum_int(vm, result);
}

static object_t scm_sub(vm_t *vm, size_t n, object_t args[]) {
  if (n < 1) {
    return make_error(vm, "-: incorrect argument count", NULL);
  }

  ASSERT_NUMBER("-", args[0])

  long result = scm_fixnum(args[0]);
  if (n == 1) {
    result = 0 - result;
  } else {
    for (int i = 1; i < n; i++) {
      ASSERT_NUMBER("-", args[i])
      result -= scm_fixnum(args[i]);
    }
  }

  return make_fixnum_int(vm, result);
}

static object_t scm_mul(vm_t *vm, size_t n, object_t args[]) {
  long result = 1;

  for (int i = 0; i < n; i++) {
    ASSERT_NUMBER("*", args[i])
    result *= scm_fixnum(args[i]);
  }

  return make_fixnum_int(vm, result);
}

#define RELATIONAL(c_name,scm_name,op) \
  static object_t c_name(vm_t *vm, size_t n, object_t args[]) { \
    if (n < 1) { \
      return make_error(vm, scm_name ": incorrect argument count", NULL); \
    } \
    ASSERT_NUMBER(scm_name, args[0]) \
    long prev = scm_fixnum(args[0]); \
    for (int i = 1; i < n; i++) { \
      ASSERT_NUMBER(scm_name, args[i]) \
      long operand = scm_fixnum(args[i]); \
      if (!(prev op operand)) return f; \
      prev = operand; \
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

