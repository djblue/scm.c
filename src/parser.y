%{
#include <stdio.h>
#include <stdlib.h>

#include "types.h"

#include "read.h"
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

%token BOOLEAN_T FIXNUM_T CHARACTER_T STRING_T SYMBOL_T EOF_T COMMENT_T

%type <obj> atom
%type <obj> vector
%type <obj> list
%type <obj> exprs
%type <obj> expr
%type <obj> quote
%type <obj> quasiquote
%type <obj> unquote
%type <obj> unquotesplice
%type <obj> comment

%%

form : expr { *obj = $1; YYACCEPT; }
     | EOF_T { *obj = eof; YYACCEPT; }
     ;

expr : atom
     | vector
     | list
     | quote
     | quasiquote
     | unquote
     | unquotesplice
     | comment
     ;

lstart : '(' { yyget_extra(scanner)->balance++; }
       ;

lend : ')' { yyget_extra(scanner)->balance--; }
     ;

bstart : '[' { yyget_extra(scanner)->balance++; }
       ;

bend : ']' { yyget_extra(scanner)->balance--; }
     ;

vector : '#' list { $$ = make_vector_from_list(vm, 1, &$2); }
       ;

list : lstart lend { $$ = NULL; }
     | bstart bend { $$ = NULL; }
     | lstart exprs lend { $$ = $2; }
     | bstart exprs bend { $$ = $2; }
     ;

exprs : expr { $$ = cons(vm, $1, NULL); }
      | expr '.' expr { $$ = cons(vm, $1, $3); }
      | expr exprs { $$ = cons(vm, $1, $2); }
      ;

comment : COMMENT_T expr { $$ = list(vm, 2, make_symbol(vm, "comment"), $2); }
        ;

quote : '\'' expr { $$ = list(vm, 2, make_symbol(vm, "quote"), $2); }
      ;

quasiquote : '`' expr { $$ = list(vm, 2, make_symbol(vm, "quasiquote"), $2); }
           ;

unquote : ',' expr { $$ = list(vm, 2, make_symbol(vm, "unquote"), $2); }
        ;

unquotesplice : ',' '@' expr { $$ = list(vm, 2, make_symbol(vm, "unquote-splicing"), $3); }
              ;

atom : BOOLEAN_T    { $$ = make_boolean(vm, yylval.str); }
     | FIXNUM_T     { $$ = make_fixnum(vm, yylval.str);  }
     | CHARACTER_T  { $$ = make_char(vm, yylval.str);    }
     | STRING_T     { $$ = make_string_internal(vm, yylval.str); }
     | SYMBOL_T     { $$ = make_symbol(vm, yylval.str);  }
     ;

%%

void yyerror(vm_t *vm, yyscan_t scanner, object_t *obj, const char *msg) {
  *obj = make_error(vm, msg, NULL);
}

