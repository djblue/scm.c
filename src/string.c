#include <string.h>

#include "string.h"
#include "env.h"
#include "symbol.h"
#include "primitive.h"
#include "number.h"
#include "character.h"
#include "error.h"

object_t make_string(vm_t *vm, size_t n, char *str) {
  object_t o = make(vm, STRING, n + 1);
  memcpy(&object_data(o, char), str, n);
  *(&object_data(o, char) + n) = '\0';
  return o;
}

object_t make_string_internal(vm_t *vm, char *str) {
  size_t n = strlen(str) - 2;
  return make_string(vm, n, str + 1);
}

char *string_cstr(object_t o) {
  return &object_data(o, char);
}

static long length(object_t str) {
  return strlen(string_cstr(str));
}

object_t string(object_t o) {
  return scm_type(o) != STRING ? f : t;
}

object_t stringp(vm_t *vm, size_t n, object_t args[]) {
  if (n != 1) {
    return make_error(vm, "string?: incorrect argument count", NULL);
  }

  return string(args[0]);
}

static object_t string_length(vm_t *vm, size_t n, object_t args[]) {
  if (n != 1) {
    return make_error(vm, "string-length: incorrect argument count", NULL);
  }

  object_t str = args[0];

  if (scm_type(str) != STRING) {
    return make_error(vm, "string-length: not a string", str);
  }

  return make_fixnum_int(vm, length(str));
}

static object_t string_ref(vm_t *vm, size_t n, object_t args[]) {
  if (n != 2) {
    return make_error(vm, "string-ref: incorrect argument count", NULL);
  }

  object_t str = args[0];
  object_t k = args[1];

  if (scm_type(str) != STRING) {
    return make_error(vm, "string-ref: not a string", str);
  }

  long len = length(str);

  if (scm_type(k) != FIXNUM) {
    return make_error(vm, "string-ref: not a number", k);
  }

  long i = scm_fixnum(k);

  if (i >= len) {
    return make_error(vm, "string-ref: out of bounds", k);
  }

  return _make_char(vm, string_cstr(str)[i]);
}

static object_t string_to_number(vm_t *vm, size_t n, object_t args[]) {
  if (n != 1) {
    return make_error(vm, "string->number: incorrect argument count", NULL);
  }

  object_t num = args[0];

  if (scm_type(num) != STRING) {
    return make_error(vm, "string->number: not a string", num);
  }

  return make_fixnum(vm, string_cstr(num));
}

void define_string(vm_t *vm, object_t env) {
  def("string?", stringp)
  def("string-length", string_length)
  def("string-ref", string_ref)
  def("string->number", string_to_number)
}
