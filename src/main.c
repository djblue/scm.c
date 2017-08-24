#include <unistd.h>

#include "vm.h"
#include "types.h"
#include "eval.h"
#include "read.h"
#include "print.h"
#include "port.h"
#include "os.h"

#include "core.xxd"

object_t load_core(vm_t *vm) {
  object_t str = make_string(vm, core_scm_len, core_scm);
  object_t port = scm_guard(scm_open_input_string(vm, 1, &str));
  object_t load = scm_load(vm, 1, &port);
  scm_unguard(port);
  return load;
}

int main (int argc, char** argv) {

  vm_t *vm = make_vm();

  object_t env = make_frame(vm);
  define_boolean(vm, env);
  define_number(vm, env);
  define_char(vm, env);
  define_string(vm, env);
  define_vector(vm, env);
  define_error(vm, env);
  define_pair(vm, env);
  define_port(vm, env);
  define_read(vm, env);
  define_print(vm, env);
  define_eval(vm, env);
  define_os(vm, env);
  assign(vm, ENV, env);

  object_t loaded = load_core(vm);

  if (loaded != t) {
    print(vm, make_error(vm, "failed to load core.scm", loaded));
    return 1;
  }

  scm_read_load(".scm_history");

  while (1) {
    object_t value = scm_read(vm, 0, NULL);
    if (value == eof) break;

    value = scm_eval(vm, value);

    if (isatty(STDIN_FILENO)) {
      print(vm, value);
    }

    vm_gc(vm);
  }

  scm_read_save(".scm_history");

  free_vm(vm);

  return 0;
}
