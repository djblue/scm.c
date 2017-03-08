#ifndef PAIR_H
#define PAIR_H

#include "object.h"

object_t *cons(vm_t *vm, object_t *car, object_t *cdr);
object_t *car(vm_t *vm, object_t *pair);
object_t *cdr(vm_t *vm, object_t *pair);
void set_car(vm_t *vm, object_t *pair, object_t *car);
void set_cdr(vm_t *vm, object_t *pair, object_t *cdr);

object_t *null(vm_t *vm, object_t *o);
object_t *pair(vm_t *vm, object_t *o);

object_t *pair_eq(vm_t *vm, object_t *a, object_t *b);

#endif
