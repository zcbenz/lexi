#ifndef REG_PARSER_H
#define REG_PARSER_H

#include "dfa.h"
#include "reg_lexer.h"

namespace lexi {

struct RegNode {
    int id;

    pos_t firstpos;
    pos_t lastpos;
    bool nullable;

    RegNode() : id(-1), nullable(true)
    {
    }

    RegNode(const set<int>& firstpos, const set<int>& lastpos,
            bool nullable)
        : id(-1), firstpos(firstpos), lastpos(lastpos), nullable(nullable)
    {
    }
};

class RegParser {
public:
    void definition(buffer_t::const_iterator begin,
                    buffer_t::const_iterator end);

    void rule(buffer_t::const_iterator begin,
              buffer_t::const_iterator end);

    void generate_dfa();
    void generate_program();

    // print followpos table and symbol_follows
    void print() const;

public:
    DFAGraph graph;

    positions_t positions;
    postable_t symbol_follows;

private:
    typedef map<string, RegNode> defis_t;

    postable_t followpos;
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
    RegLexer lex;
    Token peek;

    RegNode root;
};

} /* lexi */

#endif /* end of REG_PARSER_H */
