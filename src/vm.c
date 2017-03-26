#include "vm.h"
#include "types.h"

typedef struct alloc_t alloc_t;

struct alloc_t {
  alloc_t *next;
};

#define alloc_data(a) (object_t*)(((char*) a) + sizeof(alloc_t))

struct vm_t {
  alloc_t *root_alloc;
  size_t allocs;
  size_t threshold;
  object_t *expr;
  object_t *env;
  object_t *proc;
  object_t *stack;
  object_t *stdin;    // default input port
  object_t *stdout;   // default output port
};

object_t *fetch(vm_t *vm, reg_t reg) {
  switch (reg) {
    case EXPR:    return vm->expr;
    case ENV:     return vm->env;
    case PROC:    return vm->proc;
    case STDIN:   return vm->stdin;
    case STDOUT:  return vm->stdout;
    default:      return NULL;
  }
}

void assign(vm_t *vm, reg_t reg, object_t *value) {
  switch (reg) {
    case EXPR:    vm->expr    = value; break;
    case ENV:     vm->env     = value; break;
    case PROC:    vm->proc    = value; break;
    case STDIN:   vm->stdin   = value; break;
    case STDOUT:  vm->stdout  = value; break;
  }
}

void push(vm_t *vm, object_t *value) {
  vm->stack = cons(vm, value, vm->stack);
}

object_t *pop(vm_t *vm) {
  object_t *tmp = vm->stack;
  if (tmp == NULL) return NULL;
  vm->stack = cdr(vm, tmp);
  return car(vm, tmp);
}

object_t *popn(vm_t *vm, size_t count) {
  if (count == 0) return NULL;
  return cons(vm, pop(vm), popn(vm, count - 1));
}

static alloc_t *make_alloc(size_t n) {
  alloc_t *alloc = (alloc_t*) malloc(sizeof(alloc_t) + n);
  alloc->next = NULL;
  return alloc;
}

vm_t *make_vm() {
  vm_t *vm = malloc(sizeof(vm_t));
  vm->root_alloc = NULL;
  vm->allocs = 0;
  vm->threshold = 128;
  vm->env = NULL;
  vm->stack = NULL;
  vm->proc = NULL;

  vm->stdin = make_port_from_file(vm, stdin);
  vm->stdout = make_port_from_file(vm, stdout);
  return vm;
}

static void mark(vm_t *vm, object_t *o) {
  if (o == NULL || o->marked) return;

  o->marked = 1;

  switch (o->type) {
    case PAIR:
      mark(vm, car(vm, o));
      mark(vm, cdr(vm, o));
      break;
    case PROCEDURE:
      mark(vm, object_data(o, proc_t).body);
      mark(vm, object_data(o, proc_t).env);
      mark(vm, object_data(o, proc_t).params);
      break;
  }
}

static void sweep(vm_t *vm, alloc_t **root) {
  alloc_t **prev = root, *alloc = *root;
  while (alloc != NULL) {
    object_t *o = alloc_data(alloc);
    if (o == NULL || o->marked == 0) {
      *prev = alloc->next;
      alloc_t *tmp = alloc;
      alloc = alloc->next;
      free(tmp);
      vm->allocs--;
    } else {
      o->marked = 0;
      if (alloc->next != NULL) {
        prev = &alloc->next;
      }
      alloc = alloc->next;
    }
  }
}

void vm_gc(vm_t *vm) {
  if (vm != NULL && vm->allocs > vm->threshold) {
    mark(vm, vm->expr);
    mark(vm, vm->env);
    mark(vm, vm->proc);
    mark(vm, vm->stack);
    mark(vm, vm->stdin);
    mark(vm, vm->stdout);
    sweep(vm, &vm->root_alloc);
    vm->threshold = 2*vm->allocs;
  }
}

void free_vm(vm_t *vm) {
  if (vm != NULL) {
    sweep(vm, &vm->root_alloc);
    free(vm);
  }
}

object_t *vm_alloc(vm_t *vm, size_t n) {
  alloc_t *alloc = make_alloc(n);
  alloc->next = vm->root_alloc;
  vm->root_alloc = alloc;
  vm->allocs++;
  return alloc_data(alloc);
}

