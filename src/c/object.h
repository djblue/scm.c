#ifndef OBJECTS_H
#define OBJECTS_H

#include <stdlib.h>
#include "vm.h"

typedef enum {
  FIXNUM = 1,
  FLONUM,
  STRING,
  CHARACTER,
  SYMBOL,
  TRUE,
  FALSE,
  PAIR,
  PRIMITIVE,
  PROCEDURE,
  ERROR,
  PORT,
  ENDOFINPUT
} type_t;

extern object_t t;
extern object_t f;

#define true(o) ((o) == &t)
#define false(o) ((o) == &f)

object_t *make(vm_t *vm, type_t type, size_t n);
object_t *object_eq(vm_t *vm, object_t *a, object_t *b);
void free_object(vm_t *vm, object_t *o);

#define object_data(o,type) (*((type*)(((char*)o) + sizeof(object_t))))

#define predicate(fun,TYPE) \
  object_t *fun(object_t *o) { \
    if (o == NULL || o->type != TYPE) { \
      return &f; } \
    return &t; }

#endif
