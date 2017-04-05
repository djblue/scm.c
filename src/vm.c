#include "stdio.h"

#include "vm.h"
#include "object.h"
#include "types.h"

typedef struct alloc_t alloc_t;

struct alloc_t {
  alloc_t *next;
};

#define alloc_data(a) (object_t)(((char*) a) + sizeof(alloc_t))

struct vm_t {
  alloc_t *root_alloc;
  size_t allocs;
  size_t threshold;
  object_t expr;
  object_t env;
  object_t fun;
  object_t argl;
  object_t val;
  object_t cont;
  object_t stack[8192];
  size_t sp;
  object_t stdin;    // default input port
  object_t stdout;   // default output port
};

object_t fetch(vm_t *vm, reg_t reg) {
  switch (reg) {
    case EXPR:    return vm->expr;
    case ENV:     return vm->env;
    case FUN:     return vm->fun;
    case ARGL:    return vm->argl;
    case VAL:     return vm->val;
    case CONTINUE: return vm->cont;
    case STDIN:   return vm->stdin;
    case STDOUT:  return vm->stdout;
    default:      return NULL;
  }
}

void assign(vm_t *vm, reg_t reg, object_t value) {
  switch (reg) {
    case EXPR:    vm->expr    = value; break;
    case ENV:     vm->env     = value; break;
    case FUN:     vm->fun     = value; break;
    case ARGL:    vm->argl    = value; break;
    case VAL:     vm->val     = value; break;
    case CONTINUE: vm->cont   = value; break;
    case STDIN:   vm->stdin   = value; break;
    case STDOUT:  vm->stdout  = value; break;
  }
}

void push(vm_t *vm, object_t value) {
  if (vm->sp == 8192) {
    fprintf(stderr, "stack overflow.");
    exit(1);
  }
  vm->stack[vm->sp++] = value;
}

object_t pop(vm_t *vm) {
  if (vm->sp == 0) return NULL;
  return vm->stack[--vm->sp];
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
  vm->threshold = 4096;
  vm->env = NULL;
  vm->sp = 0;
  vm->fun = NULL;
  vm->argl = NULL;
  vm->val = NULL;
  vm->cont = NULL;

  vm->stdin = make_port_from_file(vm, stdin);
  vm->stdout = make_port_from_file(vm, stdout);
  return vm;
}

static void mark(vm_t *vm, object_t o) {
  if (o == NULL || scm_is_marked(o)) return;

  scm_mark(o);

  switch (scm_type(o)) {
    case PAIR:
      mark(vm, car(vm, o));
      mark(vm, cdr(vm, o));
      break;
    case PROCEDURE:
      mark(vm, object_data(o, proc_t).body);
      mark(vm, object_data(o, proc_t).env);
      mark(vm, object_data(o, proc_t).params);
      break;
    default:
      break;
  }
}

static void sweep(vm_t *vm, alloc_t **root) {
  alloc_t **prev = root, *alloc = *root;
  while (alloc != NULL) {
    object_t o = alloc_data(alloc);
    if ((o == NULL || !scm_is_marked(o)) && !scm_has_guard(o)) {
      *prev = alloc->next;
      alloc_t *tmp = alloc;
      alloc = alloc->next;
      free(tmp);
      vm->allocs--;
    } else {
      scm_unmark(o);
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
    for (size_t i = 0; i < vm->sp; i++) {
      mark(vm, vm->stack[i]);
    }
    mark(vm, vm->fun);
    mark(vm, vm->argl);
    mark(vm, vm->val);
    mark(vm, vm->cont);
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

object_t vm_alloc(vm_t *vm, size_t n) {
  alloc_t *alloc = make_alloc(n);
  alloc->next = vm->root_alloc;
  vm->root_alloc = alloc;
  vm->allocs++;
  return alloc_data(alloc);
}

