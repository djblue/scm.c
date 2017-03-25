#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "types.h"
#include "env.h"

#include "parser.h"
#include "lexer.h"

static object_t *read_internal(vm_t *vm, FILE *fp) {
  yyscan_t scanner;
  yylex_init(&scanner);
  yyset_in(fp, scanner);
  object_t *expr = NULL;
  yyparse(vm, scanner, &expr);
  yylex_destroy(scanner);
  return expr;
}

object_t *scm_read(vm_t *vm, object_t *args) {
  object_t *port = car(vm, args);
  if (port != NULL && port->type != PORT)
    return make_error(vm, "Provided argument is not port.");

  if (port == NULL)
    port = fetch(vm, STDIN);

  return read_internal(vm, port_pointer(port));
}

void define_read(vm_t *vm, object_t *env) {
  def("read", scm_read)
}
