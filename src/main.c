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

  object_t env = make_frame(vm);
  init(vm, env);
  define_port(vm, env);
  define_read(vm, env);
  assign(vm, ENV, env);

  FILE *core = fmemopen(core_scm, core_scm_len, "r");

  while (1) {
    object_t value = c_read(vm, core);
    if (value == eof) break;
    assign(vm, EXPR, value);
    eval(vm);
  }

  scm_read_load(".scm_history");

  while (1) {
    object_t value = scm_read(vm, NULL);
    assign(vm, EXPR, value);
    if (value == eof) break;
    eval(vm);

    if (isatty(STDIN_FILENO)) {
      print(vm, fetch(vm, VAL));
    }

    vm_gc(vm);
  }

  scm_read_save(".scm_history");

  free_vm(vm);

  return 0;
}
