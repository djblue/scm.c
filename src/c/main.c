#include "vm.h"
#include "types.h"
#include "print.h"
#include "eval.h"
#include "read.h"

#include "parser.h"
#include "lexer.h"

vm_t *vm = NULL;

int main (int argc, char** argv) {
  yyscan_t scanner;

  yylex_init(&scanner);
  yyset_in(stdin, scanner);

  vm = make_vm();

  object_t *env = make_frame(vm, NULL);
  init(vm, env);
  define_port(vm, env);
  define_read(vm, env);
  vm_set_env(vm, env);
  object_t *expr = NULL;

  if (isatty(STDIN_FILENO)) {
    printf("> ");
  }

  while (yyparse(vm, scanner, &expr) == 0 && expr != &eof) {
    object_t *value = eval(vm, expr, vm_env(vm));

    if (isatty(STDIN_FILENO)) {
      print(vm, value);
      printf("\n> ");
    }

    vm_gc(vm);
  }

  yylex_destroy(scanner);

  return 0;
}
