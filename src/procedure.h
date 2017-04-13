#ifndef PROCEDURE_H
#define PROCEDURE_H

#include "object.h"
#include "vm.h"

typedef struct {
  object_t body;
  object_t params;
  object_t env;
  int macro;
} proc_t;

object_t make_procedure(vm_t *vm, object_t env, object_t args, object_t body, int macro);

object_t procedure(object_t o);

#endif

