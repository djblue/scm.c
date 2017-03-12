#include "character.h"

object_t *make_char(vm_t *vm, char *str) {
  object_t *o = make(vm, CHARACTER, sizeof(char));
  object_data(o, char) = *(str + 2);
  return o;
}

predicate(character, CHARACTER)

object_t *character_eq(vm_t *vm, object_t *a, object_t *b) {
  return &f;
}
