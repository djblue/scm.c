#include <unistd.h>

#include "vm.h"
#include "types.h"
#include "eval.h"
#include "read.h"
#include "print.h"
#include "port.h"

#include "core.xxd"

int main (int argc, char** argv) {

  vm_t *vm = make_vm();

  object_t *env = make_frame(vm);
  init(vm, env);
  define_port(vm, env);
  define_read(vm, env);
  vm_set_env(vm, env);

  while (1) {
    if (isatty(STDIN_FILENO)) {
      printf("> ");
    }

    object_t *value = scm_read(vm, NULL);
    if (value == &eof) break;
    value = eval(vm, value, env);

    if (isatty(STDIN_FILENO)) {
      print(vm, value);
    }

    vm_gc(vm);
  }

  free_vm(vm);

  return 0;
}
