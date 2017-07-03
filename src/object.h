#ifndef OBJECTS_H
#define OBJECTS_H

#include <stdlib.h>
#include "vm.h"

typedef enum {
  NIL = 0,
  FIXNUM,
  STRING,
  VECTOR,
  CHARACTER,
  SYMBOL,
  TRUE,
  FALSE,
  PAIR,
  SPECIAL,
  PRIMITIVE,
  PROCEDURE,
  MACRO,
  ERROR,
  PORT,
  ENDOFINPUT
} type_t;

extern object_t t;
extern object_t f;

#define true(o) ((o) == t)
#define false(o) ((o) == f)

type_t scm_type(object_t o);

void scm_mark(object_t o);
void scm_unmark(object_t o);
int scm_is_marked(object_t o);
object_t scm_guard(object_t o);
object_t scm_unguard(object_t o);
int scm_has_guard(object_t o);

object_t make(vm_t *vm, type_t type, size_t n);
object_t object_eq(vm_t *vm, object_t a, object_t b);
void free_object(vm_t *vm, object_t o);

#define object_data(o,type) (*((type*)(((char*)o) + sizeof(object_t))))

#define predicate(fun,TYPE) \
  object_t fun(object_t o) { \
    if (o == NULL || scm_type(o) != TYPE) { \
      return f; } \
    return t; }

#endif
