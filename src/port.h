#ifndef PORT_H
#define PORT_H

#include <stdio.h>

#include "object.h"
#include "vm.h"

extern object_t eof;
extern object_t ueof;

object_t make_port_from_file(vm_t *vm, FILE *fp);

FILE *port_pointer(object_t port);
void print_port(vm_t *vm, object_t port);
void define_port(vm_t *vm, object_t env);

#endif
