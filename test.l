
id [a-zA-Z_][a-zA-Z0-9]* 
number [-+][0-9]*|[0-9]*|[0-9]*\.[0-9]*|[-+][0-9]*\.[0-9]*
ws [ \r\n]*

%%

{ws} 

{number} printf("%s\t\t数字\n", yytext);

#[a-zA-Z]* printf("%s\t\t宏\n", yytext);

if|else|while|do|until printf("%s\t\t逻辑保留字\n", yytext);

!=|!|=|&|\||\+|-|\*|/|^|% printf("%s\t\t运算符\n", yytext);

&&|\|\| printf("%s\t\t逻辑符\n", yytext);

\.|\'|\"|,|;|\(|\)|\||\[|\]|\{|\} printf("%s\t\t符号\n", yytext);

void|char|bool|int|enum|class|float|double|long|unsigned|union|struct printf("%s\t\t类型名\n", yytext);

{id} printf("%s\t\t变量\n", yytext);

%%

int main(int argc, char *argv[])
{
    yylex();

    return 0;
}

