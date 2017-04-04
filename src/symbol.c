#include <string.h>

#include "symbol.h"

object_t make_symbol(vm_t *vm, char *str) {
  size_t n = strlen(str) + 1;
  object_t o = make(vm, SYMBOL, n);
  memcpy(&object_data(o, char), str, n);
  return o;
}

object_t symbol_eq(vm_t *vm, object_t a, object_t b) {
  if (a == NULL || b == NULL) return f;
  if (scm_type(a) != scm_type(b)) return f;
  if (scm_type(a) != SYMBOL && scm_type(a) != STRING && scm_type(a) != ERROR) return f;
  if (strcmp(&object_data(a,char), &object_data(b,char)) != 0) return f;
  return t;
}

char *symbol_str(vm_t *vm, object_t sym) {
  return &object_data(sym, char);
}

predicate(symbol, SYMBOL)
