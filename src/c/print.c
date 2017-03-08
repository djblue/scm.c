#include <stdio.h>

#include "print.h"
#include "colors.h"

static char *get_str(object_t *o) {
  return &object_data(o, char);
}

static void print_object(vm_t *vm, object_t *o);

static void print_pair(vm_t *vm, object_t *o) {
  printf("(");

  object_t *temp = o, *next;
  while (temp != NULL) {
    print_object(vm, car(vm, temp));
    next = cdr(vm, temp);
    if (next == NULL || next->type != PAIR) break;
    printf(" ");
    temp = next;
  }

  if (temp != NULL) {
    if (next != NULL) {
      printf(" . ");
      print_object(vm, next);
    }
  }

  printf(")");
}

static void print_object(vm_t *vm, object_t *o) {
  if (o == NULL) {
    printf(__green("nil"));
  } else {
    switch (o->type) {
      case FIXNUM:
        printf(__green("%d"), object_data(o, int));
        break;
      case FLONUM:
        printf(__green("%f"), object_data(o, float));
        break;
      case CHARACTER:
        printf(__yellow("#\\%c"), object_data(o, char));
        break;
      case STRING:
        printf(__yellow("%s"), get_str(o));
        break;
      case TRUE:
        printf(__green("#t"));
        break;
      case FALSE:
        printf(__green("#f"));
        break;
      case PAIR:
        print_pair(vm, o);
        break;
      case SYMBOL:
        printf(__blue("%s"), get_str(o));
        break;
      case PROCEDURE:
        printf(__purple("#<procedure@%p>"), (void*) o);
        break;
      case PRIMITIVE:
        printf(__purple("#<primitive@%p>"), (void*) o);
        break;
      case ERROR:
        printf(__red("#<error:%s>"), get_str(o));
        break;
      case PORT:
        print_port(vm, o);
        break;
      default:
        printf("cannot write unknown type");
    }
  }
}

void print(vm_t *vm, object_t *o) {
  print_object(vm, o);
}

