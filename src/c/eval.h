#ifndef EVAL_H
#define EVAL_H

#include "types.h"
#include "env.h"

void init(vm_t *vm, object_t *env);
object_t *eval(vm_t *vm, object_t *expr, object_t **env);

#endif
