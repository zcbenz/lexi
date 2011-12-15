#include "lex.h"

#include <stdio.h>
#include <algorithm>
#include <iterator>

int main(int argc, char *argv[])
{
    lexi::buffer_t in, out;

    // read file
    FILE *file = fopen("test.l", "r+"); char ch;
    while ((ch = fgetc(file)) != EOF) in.push_back(ch);
    fclose(file);

    lexi::Lex parser(in, out);
    parser.parse();
    parser.generate_program();

    file = fopen("test.l.c", "w+");
    out.push_back('\0');
    fputs(out.data(), file);
    fclose(file);

    return 0;
}
