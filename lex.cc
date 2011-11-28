#include "lex.h"
#include <ctype.h>
#include <string.h>
#include <algorithm>

namespace lexi {

Lex::Lex(buffer_t& in, buffer_t& out)
    : state(IN_DEFINITION), in(in), out(out)
{
}

int Lex::parse()
{
    size_t i = 0;

    i = parse_definitions(i);
    if (i < 0) {
        fprintf (stderr, "Error occurred at: %d\n", -i);
        return -i;
    }

    if (state == IN_RULES) {
        i = parse_rules(i);
    } else {
        fprintf (stderr, "Rules section is required");
        return i; 
    }
}

int Lex::parse_definitions(size_t i)
{
    size_t end = getline(i);
    iterate_t it = next(i);

    for (; it != END; i = next_line(i), end = getline(i), it = next(i)) {
        // definition section ended
        if (it == NEXT) {
            state_increment();
            break;
        }

        // %{
        // find c delcaration
        // }%
        if (compare_next(i, end, "%{")) {
            i = next_line(i);
            buffer_t::iterator mark = in.begin() + i;

            // find %}
            for (; it != END; i = next_line(i), it = next(i)) {
                if (it == NEXT) return -i;

                end = getline(i);
                if (compare_next(i, end, "%}")) {
                    std::copy(mark, in.begin() + i, std::back_inserter(out));

                    i = next_line(i);
                    break;
                }
            }
        }

        // /*
        // skip comments
        // */
        i = skip_comments(i);

        // normal definitions
    }

    return i;
}

int Lex::parse_rules(size_t i)
{
    return i;
}

Lex::iterate_t Lex::next(size_t i)
{
    size_t end = in.size();
    // end of buffer
    if (i >= end)
        return END;

    // next section
    if (end - i >= 2 && in[i] == '%' && in[i + 1] == '%')
        return NEXT;

    return CONTINUE;
}

unsigned Lex::state_increment()
{
    if ((unsigned)state >= (unsigned)IN_INVALID) {
        state = IN_INVALID;
        return IN_INVALID;
    }

    state = (state_t)((unsigned)state + 1);
}

bool Lex::compare_next(size_t i, const char *mark)
{
    size_t len = strlen(mark);
    if (i + len > in.size()) return false;

    return 0 == strncmp(in.data() + i, mark, len);
}

bool Lex::compare_next(size_t i, size_t end, const char *mark)
{
    size_t len = strlen(mark);
    if (i + len > in.size()) return false;
    if (end - i != len) return false;

    return 0 == strncmp(in.data() + i, mark, len);
}

size_t Lex::getline(size_t i)
{
    while (i < in.size()) {
        if (in[i] == '\n')
            break;
        else if (i + 1 < in.size() && in[i] == '\r' && in[i + 1] == '\n')
            break;

        ++i;
    }

    return i;
}

size_t Lex::next_line(size_t i)
{
    while (i < in.size()) {
        if (in[i] == '\n')
            return i + 1;
        else if (i + 1 < in.size() && in[i] == '\r' && in[i + 1] == '\n')
            return i + 2;

        ++i;
    }

    return i;
}

size_t Lex::skip_spaces(size_t i)
{
    while (i < in.size() && isspace(in[i])) ++i;
}

size_t Lex::skip_comments(size_t i)
{
    size_t old = i;
    size_t end = getline(i);
    while (i < end) {
        if (compare_next(i, "/*")) {
            i += 2;
            while (i < in.size()) {
                if (compare_next(i, "*/")) {
                    return i + 2;
                }
                i++;
            }
        }
        i++;
    }

    return old;
}

} /* lexi */
