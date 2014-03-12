%option yylineno
%pointer

%{
    #include <stdlib.h>
    #include <string.h>
    #include "ast.hpp"
    #include "primitive.hpp"
    #include "symtab.hpp"
    #include "classhierarchy.hpp"
    #include "parser.hpp"
    
    void yyerror(const char *);
%}

/* Put your definitions here, if you have any */
INTEGER   0|[123456789]+[0123456789]*
IDMETHVAR [a-z_]+[a-zA-Z0-9_]*
IDCLASS   [A-Z]+[a-zA-Z0-9_]*
%x IN_COMMENT

%%
<INITIAL>{
"/*"              BEGIN(IN_COMMENT);
}
<IN_COMMENT>{
"*/"      BEGIN(INITIAL);
[^*\n]+
"*"
\n        yylineno;
}


Nothing { return NOTHING; }
print { return PRINT; }
if { return IF; }
then { return THEN; }
return { return RETURN; }
and { return AND; }
not { return NOT; }
Int { return INT; }
Bool { return BOOL; }
from { return FROM; }

"+" { return PLUS; }
"-" { return MINUS; }
"*" { return TIMES; }
"/" { return DIVIDE; }
"<" { return LT; }
"<=" { return LTE; }
"." { return DOT; }
"(" { return OPENPAREN; }
")" { return CLOSEPAREN; }
"{" { return OPENCURLY; }
"}" { return CLOSECURLY; }
"=" { return EQUAL; }
"," { return COMMA; }
":" { return COLON; }
";" { return SEMI; }

{INTEGER} { yylval.u_base_int = atoi(yytext); return NUM_LITERAL; }
true { yylval.u_base_int = 1; return BOOL_LITERAL; }
false { yylval.u_base_int = 0; return BOOL_LITERAL; }

{IDMETHVAR} { yylval.u_base_charptr = strdup(yytext); return IDMETHVAR; }
{IDCLASS} { yylval.u_base_charptr = strdup(yytext); return IDCLASS; }

[ \t\n]                   ; /* Put your rules with attached Lexer actions here. */

.                         { yyerror("invalid character"); }

%%

int yywrap(void) {
    return 1;
}
