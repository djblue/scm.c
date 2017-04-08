#include "number.h"
#include "error.h"

#define FIXNUM_TAG 0x1

object_t make_fixnum(vm_t *vm, char *str) {
  long value = (atol(str) << 1) | FIXNUM_TAG;
  return (object_t) value;
}

object_t make_fixnum_int(vm_t *vm, long fix) {
  return (object_t) ((fix << 1) | FIXNUM_TAG);
}

object_t make_flonum(vm_t *vm, char *str) {
  object_t o = make(vm, FLONUM, sizeof(float));
  object_data(o, float) = atof(str);
  return o;
}

object_t make_flonum_float(vm_t *vm, float flo) {
  object_t o = make(vm, FLONUM, sizeof(float));
  object_data(o, float) = flo;
  return o;
}

long scm_fixnum(object_t o) {
  return ((long) o) >> 1;
}

int scm_is_fixnum(object_t o) {
  return (((long) o) & 0x1) == FIXNUM_TAG;
}

object_t number(object_t o) {
  if (o == NULL || (scm_type(o) != FIXNUM && scm_type(o) != FLONUM)) {
    return f;
  }
  return t;
}

object_t plus(vm_t *vm, object_t a, object_t b) {
  if (a == NULL) return plus(vm, make_fixnum_int(vm, 0), b);
  if (b == NULL) return plus(vm, a, make_fixnum_int(vm, 0));

  if (!true(number(a)))
    return make_error(vm,
        "can't perform arithmetic on non numeric values", a);

  if (!true(number(b)))
    return make_error(vm,
        "can't perform arithmetic on non numeric values", b);

  if (scm_type(a) == FLONUM || scm_type(b) == FLONUM) {
    if (scm_type(a) != FLONUM) return make_flonum_float(vm, scm_fixnum(a) + object_data(b, float));
    if (scm_type(b) != FLONUM) return make_flonum_float(vm, object_data(a, float) + scm_fixnum(b));
    return make_flonum_float(vm, object_data(a, float) + object_data(b, float));
  }

  return make_fixnum_int(vm, scm_fixnum(a) + scm_fixnum(b));
}

object_t minus(vm_t *vm, object_t a, object_t b) {
  if (b == NULL) return minus(vm, make_fixnum_int(vm, 0), a);

  if (!true(number(a)))
    return make_error(vm,
        "can't perform arithmetic on non numeric values", a);

  if (!true(number(b)))
    return make_error(vm,
        "can't perform arithmetic on non numeric values", b);

  if (scm_type(a) == FLONUM || scm_type(b) == FLONUM) {
    if (scm_type(a) != FLONUM) return make_flonum_float(vm, scm_fixnum(a) - object_data(b, float));
    if (scm_type(b) != FLONUM) return make_flonum_float(vm, object_data(a, float) - scm_fixnum(b));
    return make_flonum_float(vm, object_data(a, float) - object_data(b, float));
  }

  return make_fixnum_int(vm, scm_fixnum(a) - scm_fixnum(b));
}

object_t multiply(vm_t *vm, object_t a, object_t b) {
  if (a == NULL) return multiply(vm, make_fixnum_int(vm, 1), b);
  if (b == NULL) return multiply(vm, a, make_fixnum_int(vm, 1));

  if (!true(number(a)))
    return make_error(vm,
        "can't perform arithmetic on non numeric values", a);

  if (!true(number(b)))
    return make_error(vm,
        "can't perform arithmetic on non numeric values", b);

  if (scm_type(a) == FLONUM || scm_type(b) == FLONUM) {
    if (scm_type(a) != FLONUM) return make_flonum_float(vm, scm_fixnum(a) * object_data(b, float));
    if (scm_type(b) != FLONUM) return make_flonum_float(vm, object_data(a, float) * scm_fixnum(b));
    return make_flonum_float(vm, object_data(a, float) * object_data(b, float));
  }

  return make_fixnum_int(vm, scm_fixnum(a) * scm_fixnum(b));
}

object_t fixnum(object_t o) {
  if (o == NULL || !scm_is_fixnum(o)) {
    return f;
  }
  return t;
}

predicate(flonum, FLONUM)

object_t number_eq(vm_t *vm, object_t a, object_t b) {
  if (a == NULL || b == NULL) return f;
  if (scm_type(a) != FIXNUM || scm_type(b) != FIXNUM) return f;
  return (scm_fixnum(a) == scm_fixnum(b)) ? t : f;
}

