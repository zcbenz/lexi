#include "lex.h"

#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <iterator>

int main(int argc, char **argv)
{
    const char *name = *argv;

    int verbose = 0;
    if (argc > 1 && !strcmp(argv[1], "-v")) {
        argc -= 1;
        argv++;
    }

    if (argc < 2) {
        printf("Usage: %s [-v] input [output]\n", name);
        return 1;
    }

    // parse input/output name
    char buffer[512];
    const char *input = argv[1];
    const char *output = buffer;
    if (argc > 2) {
        output = argv[2];
    } else {
        sprintf(buffer, "%s.c", input);
    }

    // read file
    lexi::buffer_t in, out;
    FILE *file = fopen(input, "r+"); char ch;
    while ((ch = fgetc(file)) != EOF) in.push_back(ch);
    fclose(file);

    // parse rules
    lexi::Lex parser(in, out);
    parser.parse();
    parser.generate_program();

    // write file
    file = fopen(output, "w+");
    out.push_back('\0');
    fputs(out.data(), file);
    fclose(file);

    return 0;
}
