#include <string.h>

#include "boolean.h"

object_t make_boolean(vm_t *vm, char *str) {
  if (strcmp(str, "#t") == 0) {
    return t;
  } else {
    return f;
  }
}

object_t boolean(object_t o) {
  if (o == t || o == f) return t;
  return f;
}

