#include "number.h"
#include "error.h"

object_t make_fixnum(vm_t *vm, char *str) {
  object_t o = make(vm, FIXNUM, sizeof(int));
  object_data(o, int) = atoi(str);
  return o;
}

object_t make_fixnum_int(vm_t *vm, int fix) {
  object_t o = make(vm, FIXNUM, sizeof(int));
  object_data(o, int) = fix;
  return o;
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

int fixnum_int(object_t o) {
  return object_data(o, int);
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

  if (!true(number(a)) || !true(number(b)))
    return make_error(vm, "can't perform arithmetic on non numeric values");

  if (scm_type(a) == FLONUM || scm_type(b) == FLONUM) {
    if (scm_type(a) != FLONUM) return make_flonum_float(vm, object_data(a, int) + object_data(b, float));
    if (scm_type(b) != FLONUM) return make_flonum_float(vm, object_data(a, float) + object_data(b, int));
    return make_flonum_float(vm, object_data(a, float) + object_data(b, float));
  }

  return make_fixnum_int(vm, object_data(a, int) + object_data(b, int));
}

object_t minus(vm_t *vm, object_t a, object_t b) {
  if (b == NULL) return minus(vm, make_fixnum_int(vm, 0), a);

  if (!true(number(a)) || !true(number(b)))
    return make_error(vm, "can't perform arithmetic on non numeric values");

  if (scm_type(a) == FLONUM || scm_type(b) == FLONUM) {
    if (scm_type(a) != FLONUM) return make_flonum_float(vm, object_data(a, int) - object_data(b, float));
    if (scm_type(b) != FLONUM) return make_flonum_float(vm, object_data(a, float) - object_data(b, int));
    return make_flonum_float(vm, object_data(a, float) - object_data(b, float));
  }

  return make_fixnum_int(vm, object_data(a, int) - object_data(b, int));
}

object_t multiply(vm_t *vm, object_t a, object_t b) {
  if (a == NULL) return multiply(vm, make_fixnum_int(vm, 1), b);
  if (b == NULL) return multiply(vm, a, make_fixnum_int(vm, 1));

  if (!true(number(a)) || !true(number(b)))
    return make_error(vm, "can't perform arithmetic on non numeric values");

  if (scm_type(a) == FLONUM || scm_type(b) == FLONUM) {
    if (scm_type(a) != FLONUM) return make_flonum_float(vm, object_data(a, int) * object_data(b, float));
    if (scm_type(b) != FLONUM) return make_flonum_float(vm, object_data(a, float) * object_data(b, int));
    return make_flonum_float(vm, object_data(a, float) * object_data(b, float));
  }

  return make_fixnum_int(vm, object_data(a, int) * object_data(b, int));
}

predicate(fixnum, FIXNUM)
predicate(flonum, FLONUM)

object_t number_eq(vm_t *vm, object_t a, object_t b) {
  if (a == NULL || b == NULL) return f;
  if (scm_type(a) != FIXNUM || scm_type(b) != FIXNUM) return f;
  return (object_data(a, int) == object_data(b, int)) ? t : f;
}

