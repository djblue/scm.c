#ifndef READ_H
#define READ_H

#include "object.h"
#include "vm.h"

typedef struct {
  vm_t *vm;
  int balance;
} extra_t;

object_t c_read(vm_t *vm, FILE *fp);
object_t scm_read(vm_t *vm, object_t args);
object_t scm_load(vm_t *vm, object_t args);
void define_read(vm_t *vm, object_t env);

void scm_read_load(const char *file);
void scm_read_save(const char *file);

#endif
