#ifndef EVAL_H
#define EVAL_H

#include "types.h"
#include "env.h"
#include "vm.h"

object_t scm_eval(vm_t *vm, object_t expr);
void define_eval(vm_t *vm, object_t env);

#endif
