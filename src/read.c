#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <readline/history.h>

#include "read.h"
#include "types.h"
#include "env.h"
#include "vm.h"
#include "eval.h"

#include "parser.h"
#include "lexer.h"

object_t c_read(vm_t *vm, FILE *fp) {
  yyscan_t scanner;
  extra_t e = { vm, 0 };
  yylex_init(&scanner);
  yylex_init_extra(&e, &scanner);
  yyset_in(fp, scanner);
  object_t expr = NULL;
  yyparse(vm, scanner, &expr);
  yylex_destroy(scanner);
  return expr;
}

object_t scm_read(vm_t *vm, size_t n, object_t args[]) {
  if (n > 1) {
    return make_error(vm, "read: incorrect argument count", NULL);
  }

  object_t port = n == 0 ? fetch(vm, STDIN) : args[0];

  if (scm_type(port) != PORT)
    return make_error(vm, "read: argument is not port", port);

  return c_read(vm, port_pointer(port));
}

object_t scm_load(vm_t *vm, size_t n, object_t args[]) {
  if (n != 1) {
    return make_error(vm, "load: incorrect argument count", NULL);
  }

  object_t port;

  if (scm_type(args[0]) == PORT) {
    port = args[0];
  } else {
    port = scm_guard(scm_open(vm, n, args));
    if (scm_type(port) == ERROR) return port;
  }

  while (1) {
    object_t value = scm_read(vm, 1, &port);
    if (value == eof) break;
    object_t ret = scm_eval(vm, value);
    if (scm_type(ret) == ERROR) {
      port = scm_unguard(port);
      scm_close(vm, 1, &port);
      return ret;
    }
  }

  port = scm_unguard(port);
  scm_close(vm, 1, &port);
  return t;
}

void scm_read_load(const char *file) {
  read_history(file);
}

void scm_read_save(const char *file) {
  write_history(file);
}

void define_read(vm_t *vm, object_t env) {
  def("read", scm_read)
  def("load", scm_load)
}
