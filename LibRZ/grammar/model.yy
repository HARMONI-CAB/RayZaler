%{
#include <string>  
#include <Vector.h>
#include <ParserContext.h>

int 
yylex(RZ::ParserContext *ctx)
{
  return ctx->lex();
}

void
yyerror(RZ::ParserContext *ctx, const char *msg)
{
  ctx->error(msg);
}

%}

%param   {RZ::ParserContext *ctx}

%define api.value.type {RZ::ValueStruct *}
%token  NUM
%token  IDENTIFIER
%token  ROTATE_KEYWORD TRANSLATE_KEYWORD PATH_KEYWORD TO_KEYWORD PARAM_KEYWORD DOF_KEYWORD
%nterm  expr

%precedence '='
%left       '-' '+'
%left       '*' '/'
%precedence NEG /* negation--unary minus */
%right      '^'      /* exponentiation */
%%

input:
    statement_set
  ;

statement_set:
    %empty
  | statement_set statement
  ;

statement:
    empty_stmt
  | ref_frame_stmt
  | element_stmt
  | path_stmt
  | dof_stmt
  | compound_stmt
  | error               { yyerrok; }
  ;

empty_stmt: ';'

element_stmt: 
    IDENTIFIER ';'
  | IDENTIFIER '(' param_list ')' ';'
  | IDENTIFIER IDENTIFIER ';'
  | IDENTIFIER IDENTIFIER '(' param_list ')' ';'


compound_stmt: '{' statement_set '}';

ref_frame_stmt:
    ref_frame_keyword '(' param_list ')' statement
  | ref_frame_keyword IDENTIFIER '(' param_list ')' statement
  ;

ref_frame_keyword: ROTATE_KEYWORD TRANSLATE_KEYWORD ;

param_list:
    %empty
  | actual_param_list
  ;


actual_param_list: 
    expr
  | assign_expr
  | actual_param_list ',' expr
  | actual_param_list ',' assign_expr
  ;

assign_expr: IDENTIFIER '=' expr ;

path_stmt:
    PATH_KEYWORD path_list ';'
  | PATH_KEYWORD IDENTIFIER path_list ';'
  ;

path_list:
    IDENTIFIER
  | path_list TO_KEYWORD IDENTIFIER
  ;

dof_stmt:
    PARAM_KEYWORD dof_decl ';'
  | DOF_KEYWORD dof_decl ';'
  ;

dof_decl:
    dof_definition
  | dof_definition '=' expr
  ;

dof_definition:
    IDENTIFIER
  | IDENTIFIER '(' expr ',' expr ')'
  ;

   expr:
     NUM
   | IDENTIFIER
   | IDENTIFIER '(' expr ')'
   | expr '+' expr       
   | expr '-' expr       
   | expr '*' expr       
   | expr '/' expr       
   | '-' expr  %prec NEG 
   | expr '^' expr       
   | '(' expr ')'        
   ;
%%
