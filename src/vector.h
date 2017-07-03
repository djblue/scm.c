#ifndef VECTOR_H
#define VECTOR_H

#include <stdio.h>

#include "object.h"
#include "vm.h"

object_t make_vector_from_list(vm_t *vm, object_t args);

long scm_vector_length(object_t vec);
object_t scm_vector_ref(object_t vec, long i);

void print_vector(vm_t *vm, FILE *fp, object_t vec);
void define_vector(vm_t *vm, object_t env);

#endif
