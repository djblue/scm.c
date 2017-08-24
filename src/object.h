#ifndef OBJECTS_H
#define OBJECTS_H

#include <stdlib.h>

typedef struct _object_t* object_t;

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

void object_init(object_t o, type_t);
type_t scm_type(object_t o);

void scm_mark(object_t o);
void scm_unmark(object_t o);
int scm_is_marked(object_t o);
object_t scm_guard(object_t o);
object_t scm_unguard(object_t o);
int scm_has_guard(object_t o);

object_t object_eq(object_t a, object_t b);

#define object_data(o,type) (*((type*)(((char*)o) + sizeof(object_t))))

#endif
