%{
#include <stdio.h>
#include <string>  
#include <Vector.h>
#include <ParserContext.h>
%}

%param   {RZ::ParserContext *ctx}

%define api.value.type {RZ::ValueType}
%token  NUM         "number"
%token  IDENTIFIER  "identifier"
%token  ROTATE_KEYWORD TRANSLATE_KEYWORD PATH_KEYWORD TO_KEYWORD PARAM_KEYWORD DOF_KEYWORD ON_KEYWORD OF_KEYWORD
%nterm  expr

%precedence '='
%left       '-' '+'
%left       '*' '/'
%precedence NEG /* negation--unary minus */
%right      '^'      /* exponentiation */
%%

input: statement_set ;

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
  | error               { YYERROR; }
  ;

empty_stmt: ';'

element_stmt: 
    IDENTIFIER ';'                                { ctx->defineElement("", $1); }
  | IDENTIFIER '(' param_list ')' ';'             { ctx->defineElement("", $1, $3); }
  | IDENTIFIER IDENTIFIER ';'                     { ctx->defineElement($2, $1); } 
  | IDENTIFIER IDENTIFIER '(' param_list ')' ';'  { ctx->defineElement($2, $1, $4); }
  ;


compound_stmt: '{' statement_set '}';             //  no-op

ref_frame_stmt:                                   // ACTION: pop
    ref_frame_decl statement { ctx->popFrame(); }
  ;

ref_frame_decl:                                   // ACTION: Push and set
    ref_frame_keyword '(' param_list ')'            { ctx->pushFrame($1, "", $3); }
  | ref_frame_keyword IDENTIFIER '(' param_list ')' { ctx->pushFrame($1, $2, $4); }
  | ON_KEYWORD IDENTIFIER OF_KEYWORD IDENTIFIER     { ctx->pushPort($4, $2);      }
  ;

ref_frame_keyword:                                // Type: enum
    ROTATE_KEYWORD         { $$ = ctx->contextType(RZ::RECIPE_CONTEXT_TYPE_ROTATION); }
  | TRANSLATE_KEYWORD      { $$ = ctx->contextType(RZ::RECIPE_CONTEXT_TYPE_TRANSLATION); };

param_list:                                       // Type: list of pairs
    %empty                             { $$ = ctx->value<RZ::ParserAssignList>(); }
  | actual_param_list
  ;


actual_param_list:                                // Type: list of pairs
    expr                               { $$ = ctx->assignExprList("", $1); }
  | assign_expr                        { $$ = ctx->assignExprList($1);     }
  | actual_param_list ',' expr         { $$ = $1; $$.value<RZ::ParserAssignList>().push_back(RZ::ParserAssignExpr("", $3)); }
  | actual_param_list ',' assign_expr  { $$ = $1; $$.value<RZ::ParserAssignList>().push_back($3); }
  ;

assign_expr: IDENTIFIER '=' expr       { $$ = ctx->assignExpr($1, $3); }
  ;

path_stmt:                                        // ACTION: register path
    PATH_KEYWORD path_list ';'            { ctx->registerPath("", $2); }
  | PATH_KEYWORD IDENTIFIER path_list ';' { ctx->registerPath($2, $3); }
  ;

path_list:                                        // Type: string list
    IDENTIFIER                        { $$ = ctx->pathList($1); }
  | path_list TO_KEYWORD IDENTIFIER   { $1.strList().push_back($3);  }
  ;

dof_stmt:                                         // ACTION: register dof or parameter
    PARAM_KEYWORD dof_decl ';' { ctx->registerParameter($2); }
  | DOF_KEYWORD dof_decl ';'   { ctx->registerDOF($2); }
  ;

dof_decl:                                         // Type: DOF pair + expr
    dof_definition
  | dof_definition '=' NUM     { $$ = $1; $$.value<RZ::ParserDOFDecl>().assign_expr = $3.str(); }
  ;

dof_definition:                                   // Type: DOF pair
    IDENTIFIER                         { $$ = ctx->dofDecl($1); }
  | IDENTIFIER '(' NUM ',' NUM ')'     { $$ = ctx->dofDecl($1, $3, $5); }
  ;

expr:                                             // Type: string
     NUM                       
   | IDENTIFIER                
   | IDENTIFIER '(' expr ')'   { $$ = $1.str() + "(" + $3.str() + ")"; }
   | expr '+' expr             { $$ = $1.str() + "+" + $3.str(); }
   | expr '-' expr             { $$ = $1.str() + "-" + $3.str(); }
   | expr '*' expr             { $$ = $1.str() + "*" + $3.str(); }
   | expr '/' expr             { $$ = $1.str() + "/" + $3.str(); }
   | '-' expr  %prec NEG       { $$ = "-" + $2.str();            }
   | expr '^' expr             { $$ = $1.str() + "^" + $3.str(); }
   | '(' expr ')'              { $$ = "(" + $1.str() + ")";      }
   ;
%%
