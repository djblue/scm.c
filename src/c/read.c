#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "types.h"
#include "eval.h"
#include "env.h"

#include "parser.h"
#include "lexer.h"

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

static object_t *read_internal(vm_t *vm, FILE *fp, object_t **env) {
  if (fp == stdin && isatty(STDIN_FILENO)) {
    n = 0;
    object_t *i = *env, *j;
    while (i != NULL) {
      j = car(vm, i);
      while (j != NULL) {
        completions[n++] = symbol_str(vm, car(vm, car(vm, j)));
        j = cdr(vm, j);
      }
      i = cdr(vm, i);
    }
    char buffer[1024];
    memset(buffer, 0, sizeof buffer);
    char *prompt = "> ";
    rl_attempted_completion_function = my_completion;
    yyscan_t scanner;
    yylex_init(&scanner);

retry:

    rl_bind_key('\t', rl_complete);
    const char *input = readline(prompt);
    if (input != NULL) {
      strcat(buffer, input);
      free(input);
    }
    if (strlen(buffer) > 0) {
      yy_scan_string(buffer, scanner);
      object_t *expr = NULL;
      yyparse(vm, scanner, &expr);
      if (expr == &ueof) {
        strcat(buffer, "\n  ");
        prompt = "  ";
        goto retry;
      }
      yylex_destroy(scanner);
      add_history(strdup(buffer));
      return expr;
    }
    if (input == NULL) {
      return &eof;
    } else {
      goto retry;
    }
  } else {
    yyscan_t scanner;
    yylex_init(&scanner);
    yyset_in(fp, scanner);
    object_t *expr = NULL;
    yyparse(vm, scanner, &expr);
    yylex_destroy(scanner);
    return expr;
  }
}

object_t *scm_read(vm_t *vm, object_t *expr, object_t **env) {
  object_t *port = eval(vm, car(vm, expr), env);
  if (port != NULL && port->type != PORT)
    return make_error(vm, "Provided argument is not port.");

  if (port == NULL)
    port = fetch(vm, STDIN);

  return read_internal(vm, port_pointer(port), env);
}

object_t *scm_load(vm_t *vm, object_t *expr, object_t **env) {
  object_t *fname = car(vm, expr);
  object_t *port = eval(vm, cons(vm, make_symbol(vm, "open"), cons(vm, fname, NULL)), env);
  FILE *fp = port_pointer(port);

  object_t *exp;
  while ((exp = read_internal(vm, fp, env)) != &eof) {
    eval(vm, exp, env);
  }

  return make_boolean(vm, "#t");
}

void define_read(vm_t *vm, object_t *env) {
  def("read", scm_read)
  def("load", scm_load)
}
