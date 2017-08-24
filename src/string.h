#ifndef STRING_H
#define STRING_H

#include "object.h"
#include "vm.h"

typedef struct {
  char *str;
  int length;
} string_t;

object_t make_string(vm_t *vm, size_t n, char *str);
object_t make_string_internal(vm_t *vm, char *str);
char *string_cstr(object_t o);
object_t str_to_error(vm_t *vm, string_t *str);

object_t string(object_t o);

void define_string(vm_t *vm, object_t env);

#endif
