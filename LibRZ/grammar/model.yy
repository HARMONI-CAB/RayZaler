%{
//
//  Copyright (c) 2024 Gonzalo José Carracedo Carballal
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as
//  published by the Free Software Foundation, either version 3 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this program.  If not, see
//  <http://www.gnu.org/licenses/>
//

#include <stdio.h>
#include <string>  
#include <Vector.h>
#include <ParserContext.h>
%}

%param   {RZ::ParserContext *ctx}

%define api.value.type {RZ::ValueType}
%token  NUM         "number"
%token  IDENTIFIER  "identifier"
%token  STRING      "string"
%token  COMP_OP     "comparison-operator"
%token  ROTATE_KEYWORD TRANSLATE_KEYWORD PORT_KEYWORD ELEMENT_KEYWORD PATH_KEYWORD TO_KEYWORD PARAM_KEYWORD DOF_KEYWORD ON_KEYWORD OF_KEYWORD IMPORT_KEYWORD SCRIPT_KEYWORD VAR_KEYWORD
%nterm  expr

%precedence '='
%left       COMP_OP
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
  | port_stmt
  | import_stmt
  | script_stmt
  | var_stmt
  | compound_stmt       
  | element_def_stmt
  | error               { YYERROR; }
  ;

empty_stmt: ';'

import_stmt:
    IMPORT_KEYWORD STRING ';'                     { ctx->import($2); }
  ;

script_stmt:
    SCRIPT_KEYWORD STRING ';'                     { ctx->script($2); }
  ;

element_stmt: 
    IDENTIFIER ';'                                { ctx->defineElement("", $1); }
  | IDENTIFIER '(' param_list ')' ';'             { ctx->defineElement("", $1, $3); }
  | IDENTIFIER IDENTIFIER ';'                     { ctx->defineElement($2, $1); } 
  | IDENTIFIER IDENTIFIER '(' param_list ')' ';'  { ctx->defineElement($2, $1, $4); }
  ;

element_def_stmt:
    element_def_decl statement                    { ctx->popElementDefinition(); }
  ;

element_def_decl:
    ELEMENT_KEYWORD IDENTIFIER                    { ctx->pushElementDefinition($2); }
  ;

compound_stmt: '{' statement_set '}';             //  no-op

port_stmt:
    PORT_KEYWORD IDENTIFIER ';'                   { ctx->pushPort($2); }
  ;

ref_frame_stmt:                                   // ACTION: pop
    ref_frame_decl statement { ctx->popFrame(); }
  ;

ref_frame_decl:                                   // ACTION: Push and set
    ref_frame_keyword '(' param_list ')'            { ctx->pushFrame($1, "", $3); }
  | ref_frame_keyword IDENTIFIER '(' param_list ')' { ctx->pushFrame($1, $2, $4); }
  | ON_KEYWORD IDENTIFIER OF_KEYWORD IDENTIFIER     { ctx->pushOnPort($4, $2);    }
  ;

ref_frame_keyword:                                // Type: enum
    ROTATE_KEYWORD         { $$ = ctx->contextType(RZ::RECIPE_CONTEXT_TYPE_ROTATION); }
  | TRANSLATE_KEYWORD      { $$ = ctx->contextType(RZ::RECIPE_CONTEXT_TYPE_TRANSLATION); };

param_list:                                       // Type: list of pairs
    %empty                             { $$ = ctx->value<RZ::ParserAssignList>(); }
  | actual_param_list
  ;


actual_param_list:                                // Type: list of pairs
    expr                               { $$ = ctx->assignExprList("", $1);   }
  | STRING                             { $$ = ctx->assignStringList("", $1); }
  | assign_expr                        { $$ = ctx->assignExprList($1);       }
  | actual_param_list ',' expr         { $$ = $1; $$.value<RZ::ParserAssignList>().push_back(RZ::ParserAssignExpr("", $3)); }
  | actual_param_list ',' assign_expr  { $$ = $1; $$.value<RZ::ParserAssignList>().push_back($3); }
  ;

assign_expr: IDENTIFIER '=' expr       { $$ = ctx->assignExpr($1, $3);   }
  |          IDENTIFIER '=' STRING     { $$ = ctx->assignString($1, $3); }
  ;

path_stmt:                                        // ACTION: register path
    PATH_KEYWORD path_list ';'            { ctx->registerPath("", $2); }
  | PATH_KEYWORD IDENTIFIER path_list ';' { ctx->registerPath($2, $3); }
  ;

path_list:                                        // Type: string list
    IDENTIFIER                        { $$ = ctx->pathList($1); }
  | path_list TO_KEYWORD IDENTIFIER   { $1.strList().push_back($3); $$ = $1; }
  ;

var_stmt: 
            VAR_KEYWORD assign_expr    { ctx->registerVariable($2); }
  ;

dof_stmt:                                         // ACTION: register dof or parameter
    PARAM_KEYWORD dof_decl ';' { ctx->registerParameter($2); }
  | DOF_KEYWORD dof_decl ';'   { ctx->registerDOF($2); }
  ;

dof_decl:                                         // Type: DOF pair + expr
    dof_definition
  | dof_definition '=' signednum     { $$ = $1; $$.value<RZ::ParserDOFDecl>().assign_expr = $3.str(); }
  ;

dof_definition:                                   // Type: DOF pair
    IDENTIFIER                                 { $$ = ctx->dofDecl($1); }
  | IDENTIFIER '(' signednum ',' signednum ')' { $$ = ctx->dofDecl($1, $3, $5); }
  ;

signednum:
        NUM
  | '-' NUM                   { $$ = "-" + $2.str();                 }
  | '+' NUM                   { $$ = $2.str();                       }
  ;
  
expr:                                             // Type: string
     NUM                       { $$ = $1; }
   | IDENTIFIER                { $$ = $1; }    
   | IDENTIFIER '(' expr ')'   { $$ = $1.str() + "(" + $3.str() + ")"; }
   | IDENTIFIER '(' expr ',' expr ')'   { $$ = $1.str() + "(" + $3.str() + "," + $5.str() + ")"; }
   | '!' expr %prec NEG        { $$ = "!" + $2.str(); }
   | expr '+' expr             { $$ = $1.str() + "+" + $3.str(); }
   | expr '-' expr             { $$ = $1.str() + "-" + $3.str(); }
   | expr '*' expr             { $$ = $1.str() + "*" + $3.str(); }
   | expr '/' expr             { $$ = $1.str() + "/" + $3.str(); }
   | expr COMP_OP expr         { $$ = $1.str() + $2.str() + $3.str(); }
   | '-' expr  %prec NEG       { $$ = "-" + $2.str();            }
   | expr '^' expr             { $$ = $1.str() + "^" + $3.str(); }
   | '(' expr ')'              { $$ = "(" + $2.str() + ")";      }
   ;
%%
