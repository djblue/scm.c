#ifndef PAIR_H
#define PAIR_H

#include "object.h"
#include "vm.h"

object_t cons(vm_t *vm, object_t car, object_t cdr);

object_t car(object_t pair);
object_t cdr(object_t pair);
object_t set_car(object_t pair, object_t car);
object_t set_cdr(object_t pair, object_t cdr);

#define caar(p) car(car (p))
#define cadr(p) car(cdr (p))
#define cdar(p) cdr(car (p))
#define cddr(p) cdr(cdr (p))

#define caaar(p) car(caar (p))
#define caadr(p) car(cadr (p))
#define cadar(p) car(cdar (p))
#define caddr(p) car(cddr (p))
#define cdaar(p) cdr(caar (p))
#define cdadr(p) cdr(cadr (p))
#define cddar(p) cdr(cdar (p))
#define cdddr(p) cdr(cddr (p))

object_t list(vm_t *vm, int argc, ...);

object_t null(object_t o);
object_t pair(object_t o);

object_t pair_eq(vm_t *vm, object_t a, object_t b);

void define_pair(vm_t *vm, object_t env);

#endif
