#include "vector.h"

#include "pair.h"
#include "error.h"
#include "boolean.h"
#include "number.h"
#include "env.h"
#include "symbol.h"
#include "primitive.h"

#include "print.h"

object_t scm_make_vector(vm_t *vm, long n) {
  object_t length = make_fixnum_int(vm, n);
  object_t zero = make_fixnum_int(vm, 0);
  size_t size = sizeof(object_t) * (n + 1);
  object_t vec = make(vm, VECTOR, size);
  (&object_data(vec, object_t))[0] = length;

  for (long i = 0; i < n; i++) {
    (&object_data(vec, object_t))[i + 1] = zero;
  }

  return vec;
}

void scm_vector_set(object_t vec, long i, object_t v) {
  (&object_data(vec, object_t))[i + 1] = v;
}

static object_t make_vector(vm_t *vm, size_t n, object_t args[]) {
  if (n != 1 && n != 2) {
    return make_error(vm, "make-vector: incorrect argument count", NULL);
  }

  object_t length = args[0];

  if (scm_type(length) != FIXNUM) {
    return make_error(vm, "make-vector: size must be a number", length);
  }

  object_t v = n == 1 ? make_fixnum_int(vm, 0) : args[1];

  long len = scm_fixnum(length);
  size_t size = sizeof(object_t) * (len + 1);
  object_t vec = make(vm, VECTOR, size);
  (&object_data(vec, object_t))[0] = length;

  for (long i = 0; i < len; i++) {
    (&object_data(vec, object_t))[i + 1] = v;
  }

  return vec;
}

static object_t vector(object_t o) {
  return scm_type(o) == VECTOR ? t : f;
}

static object_t vectorp(vm_t *vm, size_t n, object_t args[]) {
  if (n != 1) {
    return make_error(vm, "vector?: incorrect argument count", NULL);
  }

  return vector(args[0]);
}

long scm_vector_length(object_t vec) {
  return scm_fixnum(((&object_data(vec, object_t))[0]));
}

object_t scm_vector_ref(object_t vec, long i) {
  return (&object_data(vec, object_t))[i + 1];
}

static object_t vector_length(vm_t *vm, size_t n, object_t args[]) {
  if (n != 1) {
    return make_error(vm, "vector-length: incorrect argument count", NULL);
  }

  object_t vec = args[0];

  if (vector(vec) != t) {
    return make_error(vm, "vector-length: not a vector", vec);
  }

  return (&object_data(vec, object_t))[0];
}

static object_t vector_ref(vm_t *vm, size_t n, object_t args[]) {
  if (n != 2) {
    return make_error(vm, "vector-ref: incorrect argument count", NULL);
  }

  object_t vec = args[0];

  if (vector(vec) != t) {
    return make_error(vm, "vector-ref: not a vector", vec);
  }

  object_t pos = args[1];

  if (scm_type(pos) != FIXNUM) {
    return make_error(vm, "vector-ref: not a number", pos);
  }

  long i = scm_fixnum(pos);

  if (i < 0) {
    return make_error(vm, "vector-ref: index must be positive", pos);
  }

  long length = scm_fixnum((&object_data(vec, object_t))[0]);

  if (i >= length) {
    return make_error(vm, "vector-ref: index too large", pos);
  }

  return (&object_data(vec, object_t))[i + 1];
}

static object_t vector_set(vm_t *vm, size_t n, object_t args[]) {
  if (n != 3) {
    return make_error(vm, "vector-ref: incorrect argument count", NULL);
  }

  object_t vec = args[0];

  if (vector(vec) != t) {
    return make_error(vm, "vector-ref: not a vector", vec);
  }

  object_t pos = args[1];

  if (scm_type(pos) != FIXNUM) {
    return make_error(vm, "vector-ref: not a number", pos);
  }

  long i = scm_fixnum(pos);

  if (i < 0) {
    return make_error(vm, "vector-ref: index must be positive", pos);
  }

  long length = scm_fixnum((&object_data(vec, object_t))[0]);

  if (i >= length) {
    return make_error(vm, "vector-ref: index too large", pos);
  }

  object_t v = args[2];

  (&object_data(vec, object_t))[scm_fixnum(pos) + 1] = v;

  return t;
}

void print_vector(vm_t *vm, FILE *fp, object_t vec) {
  long size = scm_fixnum((&object_data(vec, object_t))[0]);

  fprintf(fp, "#(");

  for (long i = 0; i < size; i++) {
    if (i > 0) {
      fprintf(fp, " ");
    }
    print_object(vm, fp, (&object_data(vec, object_t))[i + 1]);
  }

  fprintf(fp, ")");
}

static long length(object_t args) {
  long n;

  for (n = 0; args != NULL; n++) {
    args = cdr(args);
  }

  return n;
}

object_t make_vector_from_list(vm_t *vm, size_t n, object_t args[]) {
  if (n != 1) {
    return make_error(vm, "list->vector: incorrect argument count", NULL);
  }

  object_t list = args[0];
  long len = length(list);

  size_t size = sizeof(object_t) * (len + 1);
  object_t vec = make(vm, VECTOR, size);
  (&object_data(vec, object_t))[0] = make_fixnum_int(vm, len);

  for (long i = 0; i < len; i++) {
    (&object_data(vec, object_t))[i + 1] = car(list);
    list = cdr(list);
  }

  return vec;
}

void define_vector(vm_t *vm, object_t env) {
  def("make-vector", make_vector)
  def("vector?", vectorp)
  def("vector-length", vector_length)
  def("vector-ref", vector_ref)
  def("vector-set!", vector_set)
  def("list->vector", make_vector_from_list)
}
