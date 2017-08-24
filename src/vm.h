#ifndef VM_H
#define VM_H

#include <stdlib.h>
#include "object.h"

typedef struct vm_t vm_t;

typedef enum {
  ENV,
  EXPR,
  FUN,
  ARGL,
  VAL,
  CONTINUE,
  STDIN,
  STDOUT
} reg_t;

vm_t *make_vm();
void free_vm(vm_t *vm);

int save(vm_t *vm);
void restore(vm_t *vm);

object_t fetch(vm_t *vm, reg_t reg);
void assign(vm_t *vm, reg_t reg, object_t value);
object_t *syms(vm_t *vm);

int vm_macro_expand(vm_t *vm);
void vm_set_macro_expand(vm_t *vm, int flag);

object_t vm_alloc(vm_t *vm, size_t s);
object_t make(vm_t *vm, type_t type, size_t n);

void vm_gc(vm_t *vm);
void vm_reset(vm_t *vm);

#endif
