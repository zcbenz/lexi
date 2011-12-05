#ifndef DFA_H
#define DFA_H

#include "lex.h"
#include "reg_lexer.h"

#define MAX 500
#define MAX_SYMBOL_NUMBER 0x81
#define MAX_DSTATE_NUMBER 1000
#define MAX_REGSTRING_LENGTH  500

namespace lexi {

class DState{
    static int DTransTable[MAX_DSTATE_NUMBER][MAX_SYMBOL_NUMBER];
    static int DState_counter;

public:
    int dStateID;
    set<int> positions;
    DState * next;

    DState(set<int> pos);

    int Move(int symbol){
        return DTransTable[dStateID][(int)symbol];
    }

    void SetTrans(int symbol, int tarDStateID){
        DTransTable[dStateID][(int)symbol] = tarDStateID;
    }
};

struct RegNode {
    set<int> firstpos;
    set<int> lastpos;
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

class Transfer {
public:
    Transfer(buffer_t &out) : out(out)
    {
    }

    void definition(buffer_t::const_iterator begin,
                    buffer_t::const_iterator end);

    int rule(buffer_t::const_iterator begin,
             buffer_t::const_iterator end);

    // print followpos table and symbol_follows
    void print() const;

    static vector<DState> DFAGraph;
    static map<int, string> actions;
    static bool finish_state_table[MAX_DSTATE_NUMBER];

private:
    typedef set<int> pos_t;
    typedef map<int, pos_t> postable_t;
    typedef vector<int> positions_t;

    postable_t followpos;
    postable_t symbol_follows;
    positions_t positions;

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

    Lexer lex;
    Token peek;

    typedef map<string, RegNode> defis_t;
    defis_t defis;

    DState * CheckState(set<int>);

public:
    void comput_finish();
};

} /* lexi */

#endif /* end of DFA_H */
