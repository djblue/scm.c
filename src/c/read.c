#include <stdio.h>

#include "types.h"
#include "eval.h"
#include "env.h"

#include "parser.h"
#include "lexer.h"

static object_t *read_internal(FILE *fp, object_t **env) {
  yyscan_t scanner;
  yylex_init(vm, &scanner);
  yyset_in(fp, scanner);

  object_t *expr = NULL;
  yyparse(scanner, &expr);
  yylex_destroy(scanner);

  return expr;
}

object_t *scm_read(vm_t *vm, object_t *expr, object_t **env) {
  object_t *port = eval(vm, car(vm, cdr(vm, expr)), env);
  if (port != NULL && port->type != PORT)
    return make_error(vm, "Provided argument is not port.");

  if (port == NULL) return read_internal(stdin, env);
  return read_internal(port_pointer(port), env);
}

void define_read(vm_t *vm, object_t *env) {
  def("read", scm_read)
}
