#ifndef NUMBER_H
#define NUMBER_H

#include "object.h"
#include "vm.h"

object_t make_fixnum(vm_t *vm, char *str);
object_t make_flonum(vm_t *vm, char *str);

long scm_fixnum(object_t o);
int scm_is_fixnum(object_t o);

object_t number_eq(vm_t *vm, object_t a, object_t b);

void define_number(vm_t *vm, object_t env);

#endif

