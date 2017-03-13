#include "primitive.h"

object_t *make_primitive(vm_t *vm, primitive fn, int special) {
  object_t *o = make(vm, (special == 1) ? SPECIAL : PRIMITIVE, sizeof(primitive));
  object_data(o, primitive) = fn;
  return o;
}

object_t *prim_apply(vm_t *vm, object_t *prim, object_t *args, object_t *env) {
  return (object_data(prim, primitive))(vm, args, env);
}
