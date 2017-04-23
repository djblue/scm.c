#include <stdio.h>

#include "print.h"
#include "colors.h"

static void print_object(vm_t *vm, FILE *fp, object_t o);

static void print_pair(vm_t *vm, FILE *fp, object_t o) {
  fprintf(fp, "(");

  object_t temp = o, next;
  while (temp != NULL) {
    print_object(vm, fp, car(temp));
    next = cdr(temp);
    if (next == NULL || scm_type(next) != PAIR) break;
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

static void print_object(vm_t *vm, FILE *fp, object_t o) {
  switch (scm_type(o)) {
    case ENDOFINPUT:
      fprintf(fp, __yellow("#<eof>"));
      break;
    case NIL:
      fprintf(fp, "()");
      break;
    case FIXNUM:
      fprintf(fp, __green("%ld"), scm_fixnum(o));
      break;
    case CHARACTER:
      fprintf(fp, __yellow("#\\%c"), object_data(o, char));
      break;
    case STRING:
      fprintf(fp, __yellow("\"%s\""), string_cstr(o));
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
      fprintf(fp, __blue("%s"), symbol_str(vm, o));
      break;
    case PROCEDURE:
      fprintf(fp, __purple("#<procedure@%p>"), (void*) o);
      break;
    case MACRO:
      fprintf(fp, __purple("#<macro@%p>"), (void*) o);
      break;
    case PRIMITIVE:
      fprintf(fp, __purple("#<primitive@%p>"), (void*) o);
      break;
    case SPECIAL:
      fprintf(fp, __purple("#<special@%p>"), (void*) o);
      break;
    case ERROR:
      fprintf(fp, "(");
      fprintf(fp, __red("error"));
      fprintf(fp, __yellow(" \"%s\""), scm_error_message(o));
      if (scm_error_irritant(o) != NULL) {
        fprintf(fp, " ");
        print_object(vm, fp, scm_error_irritant(o));
      }
      fprintf(fp, ")");
      break;
    case PORT:
      print_port(vm, o);
      break;
    default:
      fprintf(fp, "print: cannot write unknown type");
  }
}

void print(vm_t *vm, object_t o) {
  FILE *fp = port_pointer(fetch(vm, STDOUT));
  print_object(vm, fp, o);
  fprintf(fp, "\n");
}

