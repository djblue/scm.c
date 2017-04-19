%{
#include "types.h"
#include "vm.h"
#include "read.h"

#include "parser.h"

#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <unistd.h>

#define YY_INPUT(buf,result,max_size) \
  result = mygetinput(yyscanner, yyin, buf, max_size);

extra_t *yyget_extra(yyscan_t yyscanner);

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

static int mygetinput(yyscan_t scanner, FILE *fp, char *buf, int size) {

  if (fp == stdin && isatty(STDIN_FILENO)) {

    vm_t *vm = yyget_extra(scanner)->vm;

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

    rl_attempted_completion_function = my_completion;

    char *line;
    if (feof(fp)) return YY_NULL;
    if (yyget_extra(scanner)->balance == 0) {
      line = readline("> ");
    } else {
      line = readline(". ");
    }
    if (!line) return YY_NULL;
    if (strlen(line) > size - 2) {
      fprintf(stderr,"input line too long\n");
      return YY_NULL;
    }
    sprintf(buf,"%s\n",line);
    free(line);
    return strlen(buf);
  } else {
    int c = '*';
    int n;
    for ( n = 0; n < size && (c = getc( fp )) != EOF && c != '\n'; ++n )
      buf[n] = (char) c;
    if ( c == '\n' )
      buf[n++] = (char) c;
    if (c == EOF && ferror( fp ))
      yy_fatal_error( "input in flex scanner failed", scanner);
    return n;
  }
}

%}

%option reentrant bison-bridge
%option warn noyywrap nodefault
%option nounput
%option noinput
%option always-interactive
%option extra-type="extra_t *"

FIXNUM 0|[1-9]{DIGIT}*
FLONUM {FIXNUM}\.{DIGIT}+

LETTER [a-zA-Z]
SPECIAL_INITIAL [!$%&*/:<=>?\^_~]
SPECIAL_SUBSEQUENT [+-.@]
DIGIT [0-9]
PECULIAR_IDENTIFIER (\+|\-|\.\.\.)
INITIAL {LETTER}|{SPECIAL_INITIAL}
SUBSEQUENT {INITIAL}|{DIGIT}|{SPECIAL_SUBSEQUENT}
SYMBOL {INITIAL}{SUBSEQUENT}*|{PECULIAR_IDENTIFIER}

CHARACTER_NAME (?i:space|newline)

CHARACTER #\\({CHARACTER_NAME}|.)

STRING \"{STRING_ELEMENT}*\"
STRING_ELEMENT \\\\|\\\"|[^"\\]

LBRACE \(
RBRACE \)

BOOLEAN #t|#f

%%

;.*
("\n"|"\t"|" ")+

{FIXNUM}          { yylval->str = yytext; return FIXNUM_T; }
{FLONUM}          { yylval->str = yytext; return FLONUM_T; }
{STRING}          { yylval->str = yytext; return STRING_T; }
{SYMBOL}          { yylval->str = yytext; return SYMBOL_T; }
{CHARACTER}       { yylval->str = yytext; return CHARACTER_T; }
{BOOLEAN}         { yylval->str = yytext; return BOOLEAN_T; }

. { return *yytext; }

<<EOF>> { return EOF_T; }

%%
