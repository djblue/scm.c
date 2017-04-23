#include <string.h>

#include "boolean.h"
#include "pair.h"
#include "env.h"
#include "symbol.h"
#include "primitive.h"

object_t make_boolean(vm_t *vm, char *str) {
  if (strcmp(str, "#t") == 0) {
    return t;
  } else {
    return f;
  }
}

static object_t booleanp(vm_t *vm, object_t args) {
  object_t o = car(args);
  return (o == t || o == f) ? t : f;
}

void define_boolean(vm_t *vm, object_t env) {
  t = scm_guard(make(vm, TRUE, 0));
  f = scm_guard(make(vm, FALSE, 0));
  def("boolean?", booleanp)
}
