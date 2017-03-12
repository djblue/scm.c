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
  object_t *args = cons(vm, make_port_from_string(vm, core_scm), NULL);
  scm_load(vm, args, &env);

  scm_read_load(".scm_history");
  vm_set_env(vm, env);

  while (1) {
    object_t *value = scm_read(vm, NULL, vm_env(vm));
    if (value == &eof) break;
    value = eval(vm, value, vm_env(vm));

    if (isatty(STDIN_FILENO)) {
      print(vm, value);
    }

    vm_gc(vm);
  }

  free_vm(vm);
  scm_read_save(".scm_history");

  return 0;
}
