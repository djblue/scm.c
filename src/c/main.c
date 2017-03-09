#include <unistd.h>

#include "vm.h"
#include "types.h"
#include "eval.h"
#include "read.h"
#include "print.h"

int main (int argc, char** argv) {

  vm_t *vm = make_vm();

  object_t *env = make_frame(vm, NULL);
  init(vm, env);
  define_port(vm, env);
  define_read(vm, env);

  vm_set_env(vm, env);

  if (isatty(STDIN_FILENO)) {
    printf("> ");
  }

  while (1) {
    object_t *value = scm_read(vm, NULL, vm_env(vm));
    if (value == &eof) break;
    value = eval(vm, value, vm_env(vm));

    if (isatty(STDIN_FILENO)) {
      print(vm, value);
      printf("\n> ");
    }

    vm_gc(vm);
  }

  free_vm(vm);

  return 0;
}
