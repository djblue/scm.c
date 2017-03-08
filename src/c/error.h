#ifndef ERROR_H
#define ERROR_H

#include "object.h"
#include "string.h"

object_t *make_error(vm_t *vm, char *str);

object_t *error(vm_t *vm, object_t *o);

#endif
