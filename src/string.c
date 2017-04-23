#include <string.h>

#include "string.h"
#include "pair.h"
#include "env.h"
#include "symbol.h"
#include "primitive.h"

object_t make_string(vm_t *vm, char *str) {
  size_t n = strlen(str) + 1 - 2;
  object_t o = make(vm, STRING, n);
  memcpy(&object_data(o, char), str + 1, n);
  *(&object_data(o, char) + n - 1) = '\0';
  return o;
}

char *string_cstr(object_t o) {
  return &object_data(o, char);
}

predicate(string, STRING)

object_t stringp(vm_t *vm, object_t args) {
  return string(car(args));
}

void define_string(vm_t *vm, object_t env) {
  def("string?", stringp)
}
