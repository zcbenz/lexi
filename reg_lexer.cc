#include <ctype.h>
#include <algorithm>
#include <utility>
#include <iterator>

#include "reg_lexer.h"

namespace lexi {

static char add_slash(char c) {
    switch (c) {
    case 'n':
        return'\n';
    case 'r':
        return'\r';
    case '0':
        return'\0';
    case 't':
        return'\t';
    case 'v':
        return'\v';
    case 'b':
        return'\b';
    case 'f':
        return'\f';
    case 'a':
        return'\a';
    case '"':
        return'"';
    case '?':
        return'?';
    case '\'':
        return'\'';
    case '\\':
        return'\\';
    }

    return c;
}

string RegLexer::getDifiName()
{
    string value;

    for (; begin < end; ++begin) {
        if (isalnum(*begin) || *begin == '_')
            value.push_back(*begin);
        else
            break;
    }

    return value;
}

bool RegLexer::skip_blanks()
{
    for (; begin < end; ++begin) {
        if (isblank(*begin))
            continue;
        else 
            break;
    }

    if (*begin == '\n' || begin == end)
        return false;
    else
        return true;
}

Token RegLexer::dealChar()
{
    char value;

    switch (*begin) {
    case '\\':
        value = *(++begin);
        ++begin;
        return Token(TOKEN_CHAR, add_slash(value));

    case '$':
    case '|':
    case '*':
    case '(':
    case ')':
        return Token(*(begin++));

    default:
        return Token(TOKEN_CHAR, *(begin++));
    }
}

Token RegLexer::dealString()
{
    string value;

    for (; begin < end; ++begin) {
        if (*begin == '"') {
            ++begin;
            break;
        }

        if (*begin == '\\') {
            ++begin;

            value.push_back(add_slash(*begin));
        } else {
            value.push_back(*begin);
        }
    }

    return Token(TOKEN_STRING, value);
}

Token RegLexer::dealSet()
{
    vector<char> elems;

    for (; begin < end; ++begin) {
        if (*begin == ']') {
            ++begin;
            break;
        } else if (*begin == '-') {
            if (elems.size() < 1) {
                fprintf (stderr, "Syntax error: range expects head but got %c\n", *(begin -1));
                return Token();
            }

            char head = *(begin -1);
            ++begin;
            if (*begin <= head) {
                fprintf (stderr, "Syntax error: invalid range\n");
                return Token();
            }

            // insert a-z or 0-9 or A-Z
            for (char tail = *begin; tail > head; tail--) {
                elems.push_back(tail);
            }
        } else if (*begin =='\\') {
            ++begin;

            elems.push_back(add_slash(*begin));
        } else {
            elems.push_back(*begin);
        }
    }

    Token tok(TOKEN_SET);
    std::copy(elems.begin(), elems.end(),
              std::inserter(tok.elems, tok.elems.begin()));

    return tok;
}

Token RegLexer::dealDif()
{
    string value;

    for (; begin < end && *begin != FINISH_CHAR; ++begin) {
        if (isalnum(*begin) || *begin == '_')
            value.push_back(*begin);
        else
            break;
    }

    if (*begin != '}') {
        fprintf (stderr, "Syntax error: expected } but got %c\n", *begin);
        return Token();
    }

    ++begin;
    return Token(TOKEN_DIFI, value);
}

Token RegLexer::next()
{
    if (isspace(*begin))
        return Token(FINISH_CHAR);

    if (*begin == '"') {
        ++begin;
        return dealString();
    }

    if (*begin == '[') {
        ++begin;
        return dealSet();
    }

    if (*begin == '{') {
        ++begin;
        return dealDif();
    }

    return dealChar();
}

} /* lexi */
