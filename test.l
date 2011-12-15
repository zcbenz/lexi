
id [a-zA-Z_][a-zA-Z0-9]* 
number [-+][0-9]*|[0-9]*
ws [ \r\n]*

%%

{ws} 

if|else|while|do|until printf("Logical: %s\n", yytext);

\'|\"|,|;|\(|\)|\||\[|\]|\{|\}|&|%|\^|\*|+|=|-|! printf("Symbol: %s\n", yytext);

int|enum|class|float|double|long|unsigned|union|struct printf("Type: %s\n", yytext);

{id} printf("ID: %s\n", yytext);

{number} printf("Number: %s\n", yytext);

%%

int main(int argc, char *argv[])
{
    yylex();

    return 0;
}

