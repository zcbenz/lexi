/* End of start section */

#include <stdio.h>
#include <stdlib.h>

static FILE *yyin = NULL;

static char yytext[512] = { 0 };
static int yyleng = 0;

void yylex();

static void lexi_Action(int);

/* User's program section */
