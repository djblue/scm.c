#include <stdio.h>

#include "print.h"
#include "colors.h"

static char *get_str(object_t *o) {
  return &object_data(o, char);
}

static void print_object(vm_t *vm, FILE *fp, object_t *o);

static void print_pair(vm_t *vm, FILE *fp, object_t *o) {
  fprintf(fp, "(");

  object_t *temp = o, *next;
  while (temp != NULL) {
    print_object(vm, fp, car(vm, temp));
    next = cdr(vm, temp);
    if (next == NULL || next->type != PAIR) break;
    fprintf(fp, " ");
    temp = next;
  }

  if (temp != NULL) {
    if (next != NULL) {
      fprintf(fp, " . ");
      print_object(vm, fp, next);
    }
  }

  fprintf(fp, ")");
}

static void print_object(vm_t *vm, FILE *fp, object_t *o) {
  if (o == NULL) {
    fprintf(fp, __green("nil"));
  } else if (o == &eof) {
    fprintf(fp, __yellow("#<eof>"));
  } else {
    switch (o->type) {
      case FIXNUM:
        fprintf(fp, __green("%d"), object_data(o, int));
        break;
      case FLONUM:
        fprintf(fp, __green("%f"), object_data(o, float));
        break;
      case CHARACTER:
        fprintf(fp, __yellow("#\\%c"), object_data(o, char));
        break;
      case STRING:
        fprintf(fp, __yellow("%s"), get_str(o));
        break;
      case TRUE:
        fprintf(fp, __green("#t"));
        break;
      case FALSE:
        fprintf(fp, __green("#f"));
        break;
      case PAIR:
        print_pair(vm, fp, o);
        break;
      case SYMBOL:
        fprintf(fp, __blue("%s"), get_str(o));
        break;
      case PROCEDURE:
        fprintf(fp, __purple("#<procedure@%p>"), (void*) o);
        break;
      case PRIMITIVE:
        fprintf(fp, __purple("#<primitive@%p>"), (void*) o);
        break;
      case ERROR:
        fprintf(fp, __red("#<error:%s>"), get_str(o));
        break;
      case PORT:
        print_port(vm, o);
        break;
      default:
        fprintf(fp, "cannot write unknown type");
    }
  }
}

void print(vm_t *vm, object_t *o) {
  FILE *fp = port_pointer(fetch(vm, STDOUT));
  print_object(vm, fp, o);
}

