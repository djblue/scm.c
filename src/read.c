#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "types.h"
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
    object_t *frames = *env;
    while (frames != NULL) {
      object_t *vals = car(vm, car(vm, frames));
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
  object_t *port = car(vm, expr);
  if (port != NULL && port->type != PORT)
    return make_error(vm, "Provided argument is not port.");

  if (port == NULL)
    port = fetch(vm, STDIN);

  return read_internal(vm, port_pointer(port), env);
}

void scm_read_load(const char *file) {
  read_history(file);
}

void scm_read_save(const char *file) {
  write_history(file);
}

void define_read(vm_t *vm, object_t *env) {
  def("read", scm_read)
}
