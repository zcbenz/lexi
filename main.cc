#include "lex.h"

#include <stdio.h>
#include <algorithm>
#include <iterator>

int main(int argc, char *argv[])
{
    lexi::buffer_t in, out;

    // read file
    FILE *file = fopen("test.l", "r"); char ch;
    while ((ch = fgetc(file)) != EOF) in.push_back(ch);
    fclose(file);

    lexi::Lex(in, out).parse();

    printf ("result:\n");
    out.push_back('\0');
    fputs(out.data(), stdout);

    return 0;
}
