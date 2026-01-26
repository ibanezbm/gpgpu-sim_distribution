%define api.pure full

%code requires {
  class Configuration;
  typedef void* yyscan_t;
}

%parse-param { yyscan_t scanner } { Configuration* cf }
%lex-param   { yyscan_t scanner }

%code provides {
  int yylex(YYSTYPE* yylval, yyscan_t scanner);
}

%{
class Configuration;
typedef void* yyscan_t;

void yyerror(yyscan_t scanner, Configuration* cf, const char* msg);

void config_assign_string(Configuration* cf, const char* field, const char* value);
void config_assign_int(Configuration* cf, const char* field, int value);
void config_assign_float(Configuration* cf, const char* field, double value);

#ifdef _WIN32
#pragma warning ( disable : 4102 )
#pragma warning ( disable : 4244 )
#endif

%}

%union {
  char   *name;
  int    num;
  double fnum;
}

%token <name> STR
%token <num>  NUM
%token <fnum> FNUM

%%

commands : commands command
         | command
;

command : STR '=' STR ';'   { config_assign_string(cf, $1, $3 ); free($1); free($3); }
        | STR '=' NUM ';'   { config_assign_int(cf, $1, $3 ); free($1); }
        | STR '=' FNUM ';'  { config_assign_float(cf, $1, $3 ); free($1); }
;

%%
