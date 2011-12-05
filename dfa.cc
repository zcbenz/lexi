#include <algorithm>
#include <iterator>
#include "dfa.h"

namespace lexi {

using namespace std;

int DState::DState_counter = 0;
int DState::DTransTable[MAX_DSTATE_NUMBER][MAX_SYMBOL_NUMBER] = {{0,},};
DState* hash_table[5][8] = {{NULL,},};

DState::DState(set<int> pos){
    this -> positions = pos;
    this -> dStateID = DState_counter ++;;
    this -> next = NULL;
}

vector<DState> Transfer::DFAGraph;
bool Transfer::finish_state_table[MAX_DSTATE_NUMBER] = {false,};
map<int, string> Transfer::actions;

RegNode Transfer::E()
{
    if (debug) fprintf (stderr, "Enter E\n");

    RegNode left = C();

    if (!peek) {
        if (debug) fprintf (stderr, "End at left of E\n");
        return left;
    }

    if (peek.tag != '|') {
        return left;
    }

    peek = lex.next();

    RegNode right = E();

    if (debug) fprintf (stderr, "Leave E\n");
    return node_or(left, right);
}

RegNode Transfer::C()
{
    if (debug) fprintf (stderr, "Enter C\n");

    RegNode left = S();

    if (!peek || peek.tag == '|' || peek.tag == ')' || peek.tag == '$') {
        if (debug) fprintf (stderr, "End with S in C\n");
        return left;
    }

    RegNode right = C();

    if (!peek && debug)
        fprintf (stderr, "End at the right of C\n");
    else if (debug)
        fprintf (stderr, "Leave C\n");

    return node_cat(left, right);
}

RegNode Transfer::S()
{
    if (debug) fprintf (stderr, "Enter S\n");

    RegNode sub = F();
    if (peek && peek.tag == '*') {
        if (debug) fprintf (stderr, "Star node\n");

        peek = lex.next();
        return node_closure(sub);
    }

    if (!peek && debug)
        fprintf (stderr, "End at the end of S\n");
    else if (debug)
        fprintf (stderr, "Leave S\n");

    return sub;
}

RegNode Transfer::F()
{
    if (debug) fprintf (stderr, "Enter F\n");

    RegNode sub;

    switch (peek.tag) {
    case TOKEN_DIFI:
        if (debug) fprintf (stderr, "Leaf Defi: %s\n", peek.str.c_str());

        if (defis.find(peek.str) != defis.end()) {
            sub = defis.find(peek.str)->second;
        } else {
            if (debug) fprintf (stderr, "Not found\n");
        }

        break;

    case '(' :
        if (debug) fprintf (stderr, "Found (\n");

        peek = lex.next();
        sub = E();

        if (peek.tag != ')') {
            fprintf (stderr, "Expects ) but got %c\n", peek.tag);
            peek = Token();
            return sub;
        }

        break;

    case TOKEN_SET:
        if (debug) fprintf (stderr, "Leaf set\n");

        sub.nullable = false;
        for (set<char>::iterator i = peek.elems.begin();
             i != peek.elems.end(); ++i)
        {
            positions.push_back(*i);

            int id = positions.size();
            symbol_follows[*i].insert(id);
            sub.firstpos.insert(id);
            sub.lastpos.insert(id);
        }

        break;

    default:
        if (peek.tag == TOKEN_CHAR) {
            if (debug) fprintf (stderr, "Leaf char: %c\n", peek.ch);

            positions.push_back(peek.ch);
            symbol_follows[peek.ch].insert(positions.size());
        } else if (peek.tag == FINISH_CHAR) {
            if (debug) fprintf (stderr, "Leaf finish\n");

            positions.push_back(-1);
            symbol_follows[FINISH_SYMBOL_INDEX].insert(positions.size());
        } else {
            if (debug) fprintf (stderr, "Leaf symbol: %c\n", peek.tag);

            positions.push_back(peek.tag);
            symbol_follows[peek.tag].insert(positions.size());
        }

        sub.nullable = false;
        sub.firstpos.insert(positions.size());
        sub.lastpos.insert(positions.size());

        break;
    }

    peek = lex.next();
    if (peek.tag == FINISH_CHAR) {
        fprintf (stderr, "Peek end\n");
    }

    if (debug) fprintf (stderr, "Leave F\n");
    return sub;
}

void Transfer::definition(buffer_t::const_iterator begin,
                          buffer_t::const_iterator end)
{
    lex.init(begin, end);

    if (!lex.skip_blanks()) return;

    string name = lex.getDifiName();
    if (debug) fprintf (stderr, "Defi: %s\n", name.c_str());

    lex.skip_blanks();
    peek = lex.next();
    RegNode reg_root = E();

    defis.insert(map<string, RegNode>::value_type(name, reg_root));
}

int Transfer::rule(buffer_t::const_iterator begin,
                   buffer_t::const_iterator end)
{
    lex.init(begin, end);

    if(!lex.skip_blanks()){
        return -1;
    }

    RegNode reg_root = E();

    // Add $ at the end of rule
    peek = Token(FINISH_CHAR);
    RegNode reg_end = F();
    reg_root = node_cat(reg_root, reg_end);

    DState *start = CheckState(reg_root.firstpos);

    int i = DFAGraph.size();
    int start_state = start -> dStateID;

    for(; i < (int)DFAGraph.size(); i++){
        for(int j = 0; j < MAX_SYMBOL_NUMBER; j ++){
            vector<int> next;
            vector<int>::iterator end = next.begin();

            for(set<int>::iterator it = symbol_follows[j].begin(); it != symbol_follows[j].end(); it ++){
                if(DFAGraph.at(i).positions.count(*it) > 0){
                    vector<int>temp = next;
                    next = vector<int>(next.size()+followpos[*it].size());
                    end = set_union(temp.begin(), temp.end(), followpos[*it].begin(), followpos[*it].end(), next.begin());
                }
            }

            set<int> next_set;
            for(vector<int>::iterator it = next.begin(); it != end; it ++){

                next_set.insert(*it);
            }

            if(next_set.size() == 0){
                DFAGraph.at(i).SetTrans(j, -1);
            }
            else{
                DState * nextDS = CheckState(next_set);
                DFAGraph.at(i).SetTrans(j, nextDS -> dStateID);
            }

        }

    }

    lex.skip_blanks();
    actions.insert(map<int,string>::value_type(start_state, lex.getAction()));

    return start_state;
}

void Transfer::print() const
{
        fprintf (stderr, "----------------------------\n");
        fprintf (stderr, "|  Char  |      Positions  |\n");
        fprintf (stderr, "----------------------------\n");
    for (postable_t::const_iterator it = symbol_follows.begin();
         it != symbol_follows.end(); ++it)
    {
        char buffer[16]; int k = 0;
        for (pos_t::const_iterator j = it->second.begin();
             j != it->second.end(); ++j)
        {
            sprintf (buffer + k * 2, "%d ", *j);
            ++k;
        }

        fprintf (stderr, "|    %c   | %15s |\n", (char)it->first, buffer);
    }
        fprintf (stderr, "----------------------------\n\n");
        fprintf (stderr, "----------------------------\n");
        fprintf (stderr, "|   Pos  |        Follows  |\n");
        fprintf (stderr, "----------------------------\n");
    for (postable_t::const_iterator it = followpos.begin();
         it != followpos.end(); ++it)
    {
        char buffer[16] = { 0 }; int k = 0;
        for (pos_t::const_iterator j = it->second.begin();
             j != it->second.end(); ++j)
        {
            sprintf (buffer + k * 2, "%d ", *j);
            ++k;
        }

        fprintf (stderr, "|    %d   | %15s |\n", it->first, buffer);
    }
        fprintf (stderr, "----------------------------\n");
}

RegNode Transfer::node_or(const RegNode& left, const RegNode& right)
{
    RegNode ret;
    // nullable(c1) or nullable(c2)
    ret.nullable = left.nullable | right.nullable;
    // firstpos(c1) U firstpos(c2)
    set_union(left.firstpos.begin(), left.firstpos.end(),
              right.firstpos.begin(), right.firstpos.end(),
              std::inserter(ret.firstpos, ret.firstpos.begin()));
    // lastpos(c1) U lastpos(c2)
    set_union(left.lastpos.begin(), left.lastpos.end(),
              right.lastpos.begin(), right.lastpos.end(),
              std::inserter(ret.lastpos, ret.lastpos.begin()));

    return ret;
}

RegNode Transfer::node_cat(const RegNode& left, const RegNode& right)
{
    RegNode ret;
    // nullable(c1) and nullable(c2)
    ret.nullable = left.nullable & right.nullable;
    // if nullable(c1)
    //     firstpos(c1) U firstpos(c2)
    // else fristpos(c1)
    if (left.nullable)
        set_union(left.firstpos.begin(), left.firstpos.end(),
                  right.firstpos.begin(), right.firstpos.end(),
                  std::inserter(ret.firstpos, ret.firstpos.begin()));
    else
        ret.firstpos = left.firstpos;
    // if nullable(c2)
    //     lastpos(c1) U lastpos(c2)
    // else lastpos(c2)
    if (left.nullable)
        set_union(left.lastpos.begin(), left.lastpos.end(),
                  right.lastpos.begin(), right.lastpos.end(),
                  std::inserter(ret.lastpos, ret.lastpos.begin()));
    else
        ret.lastpos = right.lastpos;

    // lastpos(c1) followed by firstpos(c2)
    for (pos_t::const_iterator it = left.lastpos.begin();
         it != left.lastpos.end(); ++it)
    {
        pos_t& old = followpos[*it];
        set_union(old.begin(), old.end(),
                  right.firstpos.begin(), right.firstpos.end(),
                  std::inserter(old, old.end()));
    }

    return ret;
}

RegNode Transfer::node_closure(const RegNode& node)
{
    // lastpos(c) followed by firstpos(c)
    for (pos_t::const_iterator it = node.lastpos.begin();
         it != node.lastpos.end(); ++it)
    {
        pos_t& old = followpos[*it];
        set_union(old.begin(), old.end(),
                  node.firstpos.begin(), node.firstpos.end(),
                  std::inserter(old, old.end()));
    }

    return RegNode(node.firstpos, node.lastpos, true);
}

void Transfer::comput_finish(){
    for(set<int>::iterator i = symbol_follows[FINISH_SYMBOL_INDEX].begin() ; i != symbol_follows[FINISH_SYMBOL_INDEX].end(); ++i){
        for(int j = 0; j < DFAGraph.size(); ++j)
            if(DFAGraph[j].positions.count(*i) > 0){
                finish_state_table[j] = true;
            }
    }
}

DState * Transfer::CheckState(set<int> pos){
    DState * hasher = hash_table[pos.size()%5][(*pos.begin()) % 8];
    DState * previous = hasher;


    for(;hasher != NULL;previous = hasher, hasher = hasher -> next){
        if((hasher -> positions).size() == pos.size() &&
           *(hasher -> positions).begin() == *pos.begin()){
               bool flag = false;
               for(set<int>::iterator i = pos.begin(); i != pos.end(); i ++){
                   if((hasher -> positions).count(*i) <= 0){
                       flag = true;
                       break;
                   }
               }
               if(!flag){
                   return hasher;
               }
        }
    }

    DState * state = new DState(pos);

    if(previous != NULL){
        previous -> next = state;
        state -> next = NULL;
    }
    else{
        hash_table[pos.size()%5][(*pos.begin()) % 8] = state;
        state -> next = NULL;
    }
    DFAGraph.push_back(*state);
    return state;
}

} /* lexi */
