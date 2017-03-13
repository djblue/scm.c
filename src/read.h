#ifndef READ_H
#define READ_H

#include "object.h"
#include "vm.h"

object_t *scm_read(vm_t *vm, object_t *expr, object_t *env);
void scm_read_load(const char *file);
void scm_read_save(const char *file);

void define_read(vm_t *vm, object_t *env);

#endif
