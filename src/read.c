#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "types.h"
#include "env.h"
#include "vm.h"
#include "eval.h"

#include "parser.h"
#include "lexer.h"

// TODO: fix issues with overflow
static char *completions[1024];
static int n;

char *character_name_generator(const char *text, int state) {
  static int list_index, len;
  char *name;

  if (!state) {
    list_index = 0;
    len = strlen(text);
  }

  while ((name = completions[list_index++])) {
    if (strncmp(name, text, len) == 0) {
      return strdup(name);
    }
  }

  return NULL;
}

static char** my_completion(const char *text, int start, int end) {
  rl_attempted_completion_over = 1;
  return rl_completion_matches(text, character_name_generator);
}

object_t c_read(vm_t *vm, FILE *fp) {
  if (fp == stdin && isatty(STDIN_FILENO)) {
    n = 0;
    object_t frames = fetch(vm, ENV);
    // TODO: fix issue with coupling to environment representations
    while (frames != NULL) {
      object_t vals = car(vm, car(vm, frames));
      while (vals != NULL) {
        completions[n++] = symbol_str(vm, car(vm, vals));
        vals = cdr(vm, vals);
      }
      frames = cdr(vm, frames);
    }
    char buffer[1024];
    memset(buffer, 0, sizeof buffer);
    char *prompt = "> ";
    rl_attempted_completion_function = my_completion;
    yyscan_t scanner;
    yylex_init(&scanner);
    object_t expr = NULL;

retry:

    rl_bind_key('\t', rl_complete);
    const char *input = readline(prompt);

    if (input == NULL) {
      yylex_destroy(scanner);
      return eof;
    }

    if (strlen(input) == 0) {
      free(input);
      goto retry;
    }

    strcat(buffer, input);
    free(input);

    yy_scan_string(buffer, scanner);
    yyparse(vm, scanner, &expr);

    if (expr == ueof || expr == eof) {
      strcat(buffer, "\n  ");
      prompt = "  ";
      goto retry;
    }

    yylex_destroy(scanner);
    add_history(buffer);
    return expr;

  } else {
    yyscan_t scanner;
    yylex_init(&scanner);
    yyset_in(fp, scanner);
    object_t expr = NULL;
    yyparse(vm, scanner, &expr);
    yylex_destroy(scanner);
    return expr;
  }
}

object_t scm_read(vm_t *vm, object_t args) {
  object_t port = car(vm, args);
  if (port != NULL && scm_type(port) != PORT)
    return make_error(vm, "Provided argument is not port.", port);

  if (port == NULL)
    port = fetch(vm, STDIN);

  return c_read(vm, port_pointer(port));
}

object_t scm_load(vm_t *vm, object_t args) {
  object_t port = scm_open(vm, args);
  if (scm_type(port) == ERROR) return port;

  args = cons(vm, port, NULL);

  while (1) {
    object_t value = scm_read(vm, args);
    if (value == eof) break;
    object_t ret = scm_eval(vm, value);
    if (scm_type(ret) == ERROR) {
      return ret;
    }
  }

  return t;
}

void scm_read_load(const char *file) {
  read_history(file);
}

void scm_read_save(const char *file) {
  write_history(file);
}

void define_read(vm_t *vm, object_t env) {
  def("read", scm_read)
  def("load", scm_load)
}
