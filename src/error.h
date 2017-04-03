#ifndef ERROR_H
#define ERROR_H

#include "object.h"
#include "string.h"
#include "vm.h"

object_t make_error(vm_t *vm, char *str);

object_t error(object_t o);

#endif
