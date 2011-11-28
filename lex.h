#ifndef LEXI_H
#define LEXI_H

#include <vector>
#include <stdio.h>

namespace lexi {
    
typedef std::vector<char> buffer_t;

class Lex {
public:
    Lex (buffer_t& in, buffer_t& out);
    int parse();

private:
    enum state_t {
        IN_CDECLARATION,
        IN_DEFINITION,
        IN_RULES,
        IN_PROGRAM,
        IN_INVALID
    } state;

    buffer_t& in;
    buffer_t& out;

    /* parsing phases */
    // parse c-declaration and definitions section
    int parse_definitions(size_t i);

    // parse rules section
    int parse_rules(size_t i);

    // used for iterating buffer
    enum iterate_t {
        END,
        NEXT,
        CONTINUE
    };
    inline iterate_t next(size_t i);

    // move to next state
    inline unsigned state_increment();

    // check if following buffer matches mark
    bool compare_next(size_t i, const char *mark);
    bool compare_next(size_t i, size_t end, const char *mark);

    // buffer operations
    size_t getline(size_t i); /* get this line's end offset */
    size_t next_line(size_t i); /* skip to next line's start */
    size_t skip_spaces(size_t i);
    size_t skip_comments(size_t i);

/* Not to be implemented */
private:
    Lex (const Lex&);
    Lex& operator= (const Lex&);

};

} /* lexi */

#endif /* end of LEXI_H */
