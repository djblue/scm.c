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

object_t scm_read(vm_t *vm, object_t args) {
  object_t port = (scm_type(args) == PORT) ? args : car(args);

  if (port != NULL && scm_type(port) != PORT)
    return make_error(vm, "read: argument is not port", port);

  if (port == NULL)
    port = fetch(vm, STDIN);

  return c_read(vm, port_pointer(port));
}

object_t scm_load(vm_t *vm, object_t args) {
  object_t port;

  if (scm_type(car(args)) == PORT) {
    port = car(args);
  } else {
    port = scm_guard(scm_open(vm, args));
    if (scm_type(port) == ERROR) return port;
  }

  while (1) {
    object_t value = scm_read(vm, port);
    if (value == eof) break;
    object_t ret = scm_eval(vm, value);
    if (scm_type(ret) == ERROR) {
      port = scm_unguard(port);
      scm_close(vm, port);
      return ret;
    }
  }

  port = scm_unguard(port);
  scm_close(vm, port);
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
