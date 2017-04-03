%{
#include <stdio.h>
#include <stdlib.h>

#include "types.h"

#include "parser.h"
#include "lexer.h"

void yyerror(vm_t *vm, yyscan_t scanner, object_t *obj, char const *msg);
%}

%define parse.error verbose
%define api.pure full
%lex-param {void *scanner}
%parse-param {vm_t *vm} {void *scanner} {object_t *obj}

%union {
  char *str;
  object_t obj;
}

%token BOOLEAN_T FIXNUM_T FLONUM_T CHARACTER_T STRING_T SYMBOL_T

%type <obj> atom
%type <obj> list
%type <obj> exprs
%type <obj> expr
%type <obj> quote
%type <obj> quasiquote
%type <obj> unquote

%%

form : %empty { *obj = eof; YYACCEPT; }
     | expr { *obj = $1; YYACCEPT; }
     ;

expr : atom
     | list
     | quote
     | quasiquote
     | unquote
     ;

list : '(' exprs ')' { $$ = $2; }
     | '(' ')' { $$ = NULL; }
     | '[' exprs ']' { $$ = $2; }
     | '[' ']' { $$ = NULL; }
     ;

exprs : %empty { $$ = NULL; }
      | expr exprs { $$ = cons(vm, $1, $2); }
      | expr '.' expr { $$ = cons(vm, $1, $3); }
      ;

quote : '\'' expr { $$ = list(vm, 2, make_symbol(vm, "quote"), $2); }
      ;

quasiquote : '`' expr { $$ = list(vm, 2, make_symbol(vm, "quasiquote"), $2); }
           ;

unquote : ',' expr { $$ = list(vm, 2, make_symbol(vm, "unquote"), $2); }
        ;

atom : BOOLEAN_T    { $$ = make_boolean(vm, yylval.str); }
     | FIXNUM_T     { $$ = make_fixnum(vm, yylval.str);  }
     | FLONUM_T     { $$ = make_fixnum(vm, yylval.str);  }
     | CHARACTER_T  { $$ = make_char(vm, yylval.str);    }
     | STRING_T     { $$ = make_string(vm, yylval.str);  }
     | SYMBOL_T     { $$ = make_symbol(vm, yylval.str);  }
     ;

%%

void yyerror(vm_t *vm, yyscan_t scanner, object_t *obj, const char *msg) {
  if (strcmp(msg, "syntax error, unexpected $end, expecting ')'") == 0) {
    *obj = ueof;
  } else {
    fprintf(stderr, "parse error: %s\n", msg);
  }
}

