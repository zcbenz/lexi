
id [a-z]* 
ws [ \r\n]*

%%

{ws} 

{id}   printf("ID\n");

%%

int main(int argc, char *argv[])
{
    yylex();

    return 0;
}

