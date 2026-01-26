%option reentrant
%option bison-bridge
%option noyywrap
%{
class Configuration;
#include "y.tab.h"

#include <cstring>
#include <cstdlib>

static unsigned int lineno = 1;

void config_error(Configuration* cf, const char * msg, int lineno);
void yyerror(Configuration* cf, char * msg);

extern int config_input(Configuration* cf, char *, int);
#define YY_EXTRA_TYPE Configuration*

#undef YY_INPUT
#define YY_INPUT(b, r, ms)                                   \
  do {                                                       \
    Configuration* cfg = yyget_extra(yyscanner);             \
    (r) = config_input(cfg, (b), (ms));                      \
  } while (0)
%}

Digit [0-9]
Exponent [eE][+-]?{Digit}+
DblConst ({Digit}*\.)?{Digit}+{Exponent}?
StrConst [A-Za-z_\-/\.][A-Za-z0-9_\-/\.\+(\{\,)\}]*

%%

   /* Ignore comments and all spaces */

\/\/[^\n]* ;
[ \t\r]*   ;

\n         { lineno++; }

   /* Commands */

\{[A-Za-z0-9_\-\.(\{\,)\}]+(\,[A-Za-z0-9_\-\.(\{\,)\}]+)*\} { yylval_param->name = strdup( yytext ); return STR; }

-?[0-9]+     { yylval_param->num = atoi( yytext ); return NUM; }

-?[0-9]*\.[0-9]+ { yylval_param->fnum = atof( yytext ); return FNUM; }

-?{DblConst}  { yylval_param->fnum = atof( yytext ); return FNUM;}

{StrConst} { yylval_param->name = strdup( yytext ); return STR; }

.          { return yytext[0]; }

%%

void yyerror(yyscan_t scanner, Configuration* cf, const char* msg)
{
  (void)scanner;
  config_error(cf, msg, lineno);
}
