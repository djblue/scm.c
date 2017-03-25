#include "primitive.h"

object_t *make_special(vm_t *vm, special_t s) {
  object_t *o = make(vm, SPECIAL, sizeof(special_t));
  object_data(o, special_t) = s;
  return o;
}

object_t *make_primitive(vm_t *vm, primitive fn) {
  object_t *o = make(vm, PRIMITIVE, sizeof(primitive));
  object_data(o, primitive) = fn;
  return o;
}

object_t *prim_apply(vm_t *vm, object_t *prim, object_t *args, object_t *env) {
  return (object_data(prim, primitive))(vm, args);
}
