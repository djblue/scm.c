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

object_t *fetch(vm_t *vm, reg_t reg);
void assign(vm_t *vm, reg_t reg, object_t *value);
void push(vm_t *vm, object_t *value);
object_t *pop(vm_t *vm);
object_t *popn(vm_t *vm, size_t count);

object_t *vm_alloc(vm_t *vm, size_t s);

void vm_gc(vm_t *vm);

#endif
