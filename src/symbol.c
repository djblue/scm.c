#include <string.h>

#include "symbol.h"

typedef struct {
  object_t left;
  object_t right;
  char name[1];
} symbol_t;

static object_t *lookup(object_t *ptree, char *str) {
  while(*ptree != NULL) {
    int x = strcmp(str, object_data(*ptree, symbol_t).name);
    if (x == 0)
      return ptree;
    if (x < 0)
      ptree = &object_data(*ptree, symbol_t).left;
    else
      ptree = &object_data(*ptree, symbol_t).right;
  }
  return ptree;
}

object_t make_symbol_internal(vm_t *vm, char *str) {
  size_t n = strlen(str);
  object_t o = scm_guard(make(vm, SYMBOL, sizeof(symbol_t) + n));
  object_data(o, symbol_t).left = NULL;
  object_data(o, symbol_t).right = NULL;
  memcpy(&(object_data(o, symbol_t).name[0]), str, n + 1);
  return o;
}

object_t make_symbol(vm_t *vm, char *str) {
  object_t *pnode = lookup(syms(vm), str);

  if (*pnode == NULL)
    *pnode = make_symbol_internal(vm, str);

  return *pnode;
}

object_t symbol_eq(vm_t *vm, object_t a, object_t b) {
  if (a == b) return t;
  return f;
}

char *symbol_str(vm_t *vm, object_t sym) {
  return &object_data(sym, symbol_t).name[0];
}

predicate(symbol, SYMBOL)
