#ifndef PROCEDURE_H
#define PROCEDURE_H

#include "object.h"
#include "vm.h"

object_t *make_procedure(vm_t *vm, object_t *env, object_t *args, object_t *body);

object_t *procedure(object_t *o);

object_t *proc_apply(vm_t *vm, object_t *procedure, object_t *args, object_t **env);

#endif

