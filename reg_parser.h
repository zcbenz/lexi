#ifndef REG_PARSER_H
#define REG_PARSER_H

#include "dfa.h"
#include "reg_lexer.h"

namespace lexi {

struct RegNode {
    pos_t firstpos;
    pos_t lastpos;
    bool nullable;

    RegNode() : nullable(true)
    {
    }

    RegNode(const set<int>& firstpos, const set<int>& lastpos,
            bool nullable)
        : firstpos(firstpos), lastpos(lastpos), nullable(nullable)
    {
    }
};

class RegParser {
public:
    RegParser(buffer_t &out) : out(out)
    {
    }

    void definition(buffer_t::const_iterator begin,
                    buffer_t::const_iterator end);

    int rule(buffer_t::const_iterator begin,
             buffer_t::const_iterator end);

    // print followpos table and symbol_follows
    void print() const;

private:
    typedef map<string, RegNode> defis_t;

    postable_t followpos;
    postable_t symbol_follows;
    positions_t positions;
    defis_t defis;

    RegNode E();
    RegNode C();
    RegNode S();
    RegNode F();

    // c1 | c2
    RegNode node_or(const RegNode& left, const RegNode& right);
    // c1 c2
    RegNode node_cat(const RegNode& left, const RegNode& right);
    // c1*
    RegNode node_closure(const RegNode& node);

private:
    buffer_t &out;

    RegLexer lex;
    Token peek;

    DFAGraph graph;
};

} /* lexi */

#endif /* end of REG_PARSER_H */
