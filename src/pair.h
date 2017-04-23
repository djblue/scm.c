#ifndef PAIR_H
#define PAIR_H

#include "object.h"
#include "vm.h"

object_t cons(vm_t *vm, object_t car, object_t cdr);
object_t car(vm_t *vm, object_t pair);
object_t cdr(vm_t *vm, object_t pair);
object_t set_car(vm_t *vm, object_t pair, object_t car);
object_t set_cdr(vm_t *vm, object_t pair, object_t cdr);

object_t list(vm_t *vm, int argc, ...);

object_t null(object_t o);
object_t pair(object_t o);

object_t pair_eq(vm_t *vm, object_t a, object_t b);

void define_pair(vm_t *vm, object_t env);

#endif
