#ifndef PRINT_H
#define PRINT_H

#include "types.h"
#include "vm.h"

void print(vm_t *vm, object_t o);

void define_print(vm_t *vm, object_t env);

#endif
