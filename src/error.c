#include <string.h>

#include "error.h"

object_t make_error (vm_t *vm, char *str) {
  size_t n = strlen(str) + 1;
  object_t o = make(vm, ERROR, n);
  memcpy(&object_data(o, char), str, n);
  return o;
}

predicate(error, ERROR)

