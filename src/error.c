#include <string.h>

#include "error.h"

typedef struct {
  object_t irritant;
  char message[1];
} error_t;

object_t make_error(vm_t *vm, char *str, object_t irritant) {
  size_t n = strlen(str);
  object_t o = make(vm, ERROR, sizeof(error_t) + n);
  memcpy(&object_data(o, error_t).message[0], str, n + 1);
  object_data(o, error_t).irritant = irritant;
  return o;
}

char *scm_error_message(object_t error) {
  return &object_data(error, error_t).message[0];
}

object_t scm_error_irritant(object_t error) {
  return object_data(error, error_t).irritant;
}

predicate(error, ERROR)

