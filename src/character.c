#include "character.h"

#include "pair.h"
#include "env.h"
#include "symbol.h"
#include "primitive.h"

object_t make_char(vm_t *vm, char *str) {
  object_t o = make(vm, CHARACTER, sizeof(char));
  object_data(o, char) = *(str + 2);
  return o;
}

object_t character_eq(vm_t *vm, object_t a, object_t b) {
  return f;
}

static object_t characterp(vm_t *vm, object_t args) {
  return scm_type(car(args)) == CHARACTER ? t : f;
}

void define_char(vm_t *vm, object_t env) {
  def("char?", characterp)
}
