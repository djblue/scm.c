#ifndef VM_H
#define VM_H

#include <stdlib.h>

typedef struct vm_t vm_t;

typedef struct object_t object_t;

struct object_t {
  unsigned char type;
  unsigned char trace;
  unsigned char marked;
  unsigned char padding;
};

vm_t *make_vm();
void free_vm(vm_t *vm);

object_t *vm_alloc(vm_t *vm, size_t s);

object_t **vm_env(vm_t *vm);
void vm_set_env(vm_t *vm, object_t *env);

void vm_gc(vm_t *vm);

#endif
