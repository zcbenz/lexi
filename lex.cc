#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>

#include "lex.h"

namespace lexi {

Lex::Lex(buffer_t in, buffer_t& out)
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

    if (state == IN_PROGRAM) {
        std::copy(in.begin() + i, in.end(), std::back_inserter(main));
    }

    t.generate_dfa();
//    t.print();
}

void Lex::generate_program()
{
    file_to_buffer("1.tpl");
    std::copy(header.begin(), header.end(), std::back_inserter(out));
    file_to_buffer("2.tpl");
    std::copy(main.begin(), main.end(), std::back_inserter(out));
    file_to_buffer("3.tpl");
    DTran_to_buffer();
    FinishStates_to_buffer();
    file_to_buffer("4.tpl");
    Actions_to_buffer();
    file_to_buffer("5.tpl");
}

int Lex::parse_definitions(size_t i)
{
    size_t end = getline(i);
    iterate_t it = next(i);

    for (; it != END; i = next_line(i), end = getline(i), it = next(i)) {
        // definition section ended
        if (it == NEXT) {
            state_increment();
            i = next_line(i);
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
                    std::copy(mark, in.begin() + i, std::back_inserter(header));

                    i = next_line(i);
                    end = getline(i);
                    break;
                }
            }
        }

        // normal definitions
        if (i < end)
            t.definition(in.begin() + i, in.begin() + end);
    }

    return i;
}

int Lex::parse_rules(size_t i)
{
    size_t end = getline(i);
    iterate_t it = next(i);

    for (; it != END; i = next_line(i), end = getline(i), it = next(i)) {
        if (it == NEXT) {
            state_increment();
            i = next_line(i);
            break;
        }

        if (i < end) {
            buffer_t::iterator ibegin = in.begin() + i;
            buffer_t::iterator iend = in.begin() + end;

            // TODO
            // expand multiple lines for {
            // }

            i = iend - in.begin();
            t.rule(ibegin, iend);
        }
    }

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

void Lex::increment(size_t &i)
{
    ++i;
    i = skip_comments(i);
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

size_t Lex::get_id(size_t i)
{
    while (i < in.size() && (isalnum(in[i]) || in[i] == '_'))
        ++i;

    return i;
}

size_t Lex::skip_spaces(size_t i)
{
    while (i < in.size() && isblank(in[i])) ++i;
}

size_t Lex::skip_comments(size_t i)
{
    i = skip_spaces(i);

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

void Lex::file_to_buffer(const char *name)
{
    FILE *file = fopen(name, "r+"); char ch;
    while ((ch = fgetc(file)) != EOF) out.push_back(ch);
    fclose(file);
}

void Lex::string_to_buffer(const char *str)
{
    while(*str) out.push_back(*str++);
}

void Lex::vector_to_buffer(const vector<int>& vec)
{
    char buffer[64];
    for (int i = 0; i < vec.size(); i++) {
        snprintf(buffer, 64, "%2d, ", vec[i]);
        string_to_buffer(buffer);

        if ((i + 1) % 8 == 0) out.push_back('\n');
    }
    out.push_back('\n');
}

void Lex::DTran_to_buffer()
{
    char buffer[512];
    const char *head = "static int lexi_DTran[%d][%d] = {\n";
    const char *tail = "};\n\n";

    snprintf(buffer, 512, head, t.graph.DStates.size(), MAX_SYMBOL_NUMBER);
    string_to_buffer(buffer);

    for (int i = 0; i < t.graph.DStates.size(); i++) {
        snprintf(buffer, 512, "{\n");
        string_to_buffer(buffer);

        vector<int> plain(MAX_SYMBOL_NUMBER, -1);
        const M2D_t& Dtran = t.graph.Dtran[i];
        for (M2D_t::const_iterator it = Dtran.begin(); it != Dtran.end(); ++it)
        {
            plain[it->first] = it->second;
        }
        vector_to_buffer(plain);

        snprintf(buffer, 512, "},\n");
        string_to_buffer(buffer);
    }

    string_to_buffer(tail);
}

void Lex::FinishStates_to_buffer()
{
    vector<int> finishes(t.graph.DStates.size(), 0);

    const pos_t& end_symbol_positions = t.symbol_follows[-1];

    for (pos_t::const_iterator it = end_symbol_positions.begin();
         it != end_symbol_positions.end(); ++it)
    {
        for (int i = 0; i < t.graph.DStates.size(); i++) {
            if (t.graph.DStates[i].positions.find(*it) !=
                t.graph.DStates[i].positions.end())
            {
                finishes[i] = 1;
            }
        }
    }

    char buffer[64];
    const char *head = "static int lexi_Finished[%d] = {\n";
    const char *tail = "};\n\n";
    snprintf(buffer, 64, head, finishes.size());

    string_to_buffer(buffer);
    vector_to_buffer(finishes);
    string_to_buffer(tail);
}

void Lex::Actions_to_buffer()
{
    set<int> bound_states;

    char buffer[64];
    const char *head = "case %d:\n";
    const char *tail = "\nbreak;\n";

    const pos_t& end_symbol_positions = t.symbol_follows[-1];

    for (pos_t::const_iterator it = end_symbol_positions.begin();
         it != end_symbol_positions.end(); ++it)
    {
        for (int i = 0; i < t.graph.DStates.size(); i++) {
            if (t.graph.DStates[i].positions.find(*it) !=
                t.graph.DStates[i].positions.end())
            {
                // check if added before
                if (bound_states.count(i) > 0)
                    continue;

                // mark state
                bound_states.insert(i);

                // print action
                snprintf(buffer, 64, head, i);
                string_to_buffer(buffer);
                std::copy(t.graph.actions[*it].begin(),
                          t.graph.actions[*it].end(),
                          std::back_inserter(out));
                string_to_buffer(tail);
            }
        }
    }
}

} /* lexi */
