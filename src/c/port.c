#include <stdio.h>

#include "types.h"
#include "colors.h"
#include "env.h"
#include "eval.h"
#include "read.h"

object_t eof = { ENDOFINPUT };
object_t ueof = { UENDOFINPUT };

typedef struct {
  object_t *path;
  FILE *fp;
} port_t;

object_t *make_port(vm_t *vm) {
  return make(vm, PORT, sizeof(port_t));
}

object_t *make_port_from_file(vm_t *vm, FILE *fp) {
  object_t * port = make_port(vm);
  object_data(port, port_t).fp = fp;
  return port;
}

FILE *port_pointer(object_t *port) {
  return object_data(port, port_t).fp;
}

void print_port(vm_t *vm, object_t *port) {
  printf(__yellow("#<port:%s>"), string_cstr(object_data(port, port_t).path));
}

defn(eval_open) {
  object_t *str = car(vm, expr);

  if (false(string(str)))
    return make_error(vm, "First argument is not a string.");

  char *path = string_cstr(str);
  FILE *fp = fopen(path, "r");

  if (fp == NULL) {
    return make_error(vm, "Can't open input file.");
  }

  object_t *port = make_port(vm);
  object_data(port, port_t).path = str;
  object_data(port, port_t).fp = fp;
  return port;
}

object_t *scm_load(vm_t *vm, object_t *expr, object_t **env) {
  object_t *args = cons(vm, eval_open(vm, expr, env), NULL);

  object_t *exp;
  while ((exp = scm_read(vm, args, env)) != &eof) {
    eval(vm, exp, env);
  }

  return make_boolean(vm, "#t");
}


void define_port(vm_t *vm, object_t *env) {
  def("open", eval_open)
  def("load", scm_load)
}

