#include <stdio.h>
#include <string.h>

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
  object_t *port = make_port(vm);
  object_data(port, port_t).fp = fp;
  return port;
}

FILE *port_pointer(object_t *port) {
  return object_data(port, port_t).fp;
}

void print_port(vm_t *vm, object_t *port) {
  printf(__yellow("#<port:%s>"), string_cstr(object_data(port, port_t).path));
}

object_t *eval_open(vm_t *vm, object_t *args) {
  object_t *str = car(vm, args);

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

void define_port(vm_t *vm, object_t *env) {
  def("open", eval_open)
}

