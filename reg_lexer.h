#include "lex.h"

#include <set>
#include <string>
#include <map>

namespace lexi {
    
using std::set;
using std::string;
using std::map;

enum TOKEN {
    TOKEN_SET = 0x100,
    TOKEN_DIFI,
    TOKEN_STRING,
    TOKEN_CHAR
};

static const int FINISH_CHAR         = '$';
static const int FINISH_SYMBOL_INDEX = -1;

struct Token {
    int tag;

    char ch;
    string str;
    set<char> elems;

    explicit Token(int tag = FINISH_CHAR) : tag(tag) {
    }

    explicit Token(int tag, char ch) : tag(tag), ch(ch) {
    }

    explicit Token(const set<char>& elems) : tag(TOKEN_SET), elems(elems) {
    }

    explicit Token(int tag, string str) : tag(tag), str(str) {
    }

    operator bool() {
        return tag != FINISH_CHAR;
    }
};

class RegLexer {
public:
    void init(buffer_t::const_iterator begin,
              buffer_t::const_iterator end)
    {
        this->begin = begin;
        this->end = end;
    }

    Token next();
    bool skip_blanks();

    string getDifiName();

    buffer_t get_action() {
        return buffer_t(begin, end);
    }

private:
    buffer_t::const_iterator begin, end;

    Token dealChar();
    Token dealSet();
    Token dealDif();
    Token dealString();
};

} /* lexi */
