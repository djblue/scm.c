#ifndef READ_H
#define READ_H

#include "object.h"
#include "vm.h"


object_t *c_read(vm_t *vm, FILE *fp);
object_t *scm_read(vm_t *vm, object_t *args);
void define_read(vm_t *vm, object_t *env);

#endif
