#ifndef ERROR_H
#define ERROR_H

#include "object.h"
#include "string.h"
#include "vm.h"

object_t make_error(vm_t *vm, char *str, object_t irritant);

char *scm_error_message(object_t error);
object_t scm_error_irritant(object_t error);

void define_error(vm_t *vm, object_t env);

#endif
