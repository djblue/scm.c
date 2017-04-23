#ifndef PORT_H
#define PORT_H

#include <stdio.h>

#include "object.h"
#include "vm.h"

extern object_t eof;

object_t make_port_from_file(vm_t *vm, FILE *fp);
object_t scm_open(vm_t *vm, object_t args);
object_t scm_close(vm_t *vm, object_t args);

FILE *port_pointer(object_t port);
void print_port(vm_t *vm, object_t port);
void define_port(vm_t *vm, object_t env);

#endif
