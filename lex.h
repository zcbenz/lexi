#ifndef LEXI_H
#define LEXI_H

#include "buffer.h"
#include "reg_parser.h"

#define MAX_SYMBOL_NUMBER 129

namespace lexi {

class Lex {
public:
    Lex (buffer_t in, buffer_t& out);

    int parse();

    // generate program
    void generate_program();

private:
    enum state_t {
        IN_CDECLARATION,
        IN_DEFINITION,
        IN_RULES,
        IN_PROGRAM,
        IN_INVALID
    } state;

    buffer_t& out;
    buffer_t in;

    RegParser t;

    buffer_t header, main;

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
    inline void increment(size_t &i);

    // move to next state
    inline unsigned state_increment();

    // check if following buffer matches mark
    bool compare_next(size_t i, const char *mark);
    bool compare_next(size_t i, size_t end, const char *mark);

    // buffer operations
    size_t getline(size_t i); /* get this line's end offset */
    size_t next_line(size_t i); /* skip to next line's start */
    size_t get_id(size_t i);
    size_t skip_spaces(size_t i);
    size_t skip_comments(size_t i);

    // write file to buffer
    void file_to_buffer(const char *file);
    void string_to_buffer(const char *str);
    void vector_to_buffer(const vector<int>& vec);

    // push struct to buffer
    void DTran_to_buffer();
    void FinishStates_to_buffer();
    void Actions_to_buffer();

/* Not to be implemented */
private:
    Lex (const Lex&);
    Lex& operator= (const Lex&);

};

} /* lexi */

#endif /* end of LEXI_H */
