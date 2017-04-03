#ifndef NUMBER_H
#define NUMBER_H

#include "object.h"
#include "vm.h"

object_t make_fixnum(vm_t *vm, char *str);
object_t make_fixnum_int(vm_t *vm, int fix);
object_t make_flonum(vm_t *vm, char *str);
object_t make_flonum_float(vm_t *vm, float flo);

int fixnum_int(object_t o);

object_t fixnum(object_t o);
object_t flonum(object_t o);
object_t number(object_t o);

object_t plus(vm_t *vm, object_t a, object_t b);
object_t minus(vm_t *vm, object_t a, object_t b);
object_t multiply(vm_t *vm, object_t a, object_t b);

object_t number_eq(vm_t *vm, object_t a, object_t b);

#endif

