#include <algorithm>
#include <utility>
#include <iterator>

#include "reg_parser.h"

namespace lexi {

using std::set_union;
using std::make_pair;

void RegParser::definition(buffer_t::const_iterator begin,
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

int RegParser::rule(buffer_t::const_iterator begin,
                   buffer_t::const_iterator end)
{
    lex.init(begin, end);

    if(!lex.skip_blanks()) {
        return -1;
    }

    peek = lex.next();
    RegNode reg_root = E();

    // Add $ at the end of rule
    peek = Token(FINISH_CHAR);
    RegNode reg_end = F();
    reg_root = node_cat(reg_root, reg_end);

    size_t i = graph.DStates.size();
    size_t start = graph.push_back(reg_root.firstpos)->id;

    for (; i < graph.DStates.size(); ++i) {
        DState& state = graph.DStates[i];

        // spit state.positions into group[a,b,c][1,2,3]
        postable_t group;
        for (pos_t::const_iterator it = state.positions.begin();
             it != state.positions.end(); ++it)
        {
            // group[a].insert(1)
            group[positions[*it]].insert(*it);
        }

        // for (a: group)
        //     positions = U follows(group[a])
        for (postable_t::const_iterator it = group.begin();
             it != group.end(); ++it)
        {
            int path = it->first;
            if (path == -1) {
                continue;
            }

            pos_t next;
            for (pos_t::const_iterator p = it->second.begin();
                 p != it->second.end(); ++p)
            {
                pos_t follow = followpos[*p];
                set_union(next.begin(), next.end(),
                          follow.begin(), follow.end(),
                          std::inserter(next, next.begin()));
            }

            // Dtran[A, a] = positions
            if (next.size() > 0) {
                graph.Dtran[state.id][path] = graph.push_back(next)->id;
            } else {
                graph.Dtran[state.id][path] = -1;
            }
        }
    }

    lex.skip_blanks();
    graph.actions.insert(make_pair(start, lex.get_action()));

    return start;
}

RegNode RegParser::E()
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

RegNode RegParser::C()
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

RegNode RegParser::S()
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

RegNode RegParser::F()
{
    if (debug) fprintf (stderr, "Enter F\n");

    RegNode sub;
    int id;

    switch (peek.tag) {
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

    case TOKEN_DIFI:
        if (debug) fprintf (stderr, "Leaf Defi: %s\n", peek.str.c_str());

        if (defis.find(peek.str) != defis.end()) {
            sub = defis.find(peek.str)->second;
        } else {
            if (debug) fprintf (stderr, "Not found\n");
        }

        break;

    case TOKEN_SET:
        if (debug) fprintf (stderr, "Leaf set\n");

        sub.nullable = false;
        for (set<char>::iterator i = peek.elems.begin();
             i != peek.elems.end(); ++i)
        {
            int id = positions.size();
            positions.push_back(*i);

            symbol_follows[*i].insert(id);
            sub.firstpos.insert(id);
            sub.lastpos.insert(id);
        }

        break;

    default:
        id = positions.size();

        if (peek.tag == TOKEN_CHAR) {
            if (debug) fprintf (stderr, "Leaf char: %c\n", peek.ch);

            symbol_follows[peek.ch].insert(id);
            positions.push_back(peek.ch);
        } else if (peek.tag == FINISH_CHAR) {
            if (debug) fprintf (stderr, "Leaf finish\n");

            symbol_follows[-1].insert(id);
            positions.push_back(-1);
        } else {
            if (debug) fprintf (stderr, "Leaf symbol: %c\n", peek.tag);

            symbol_follows[peek.tag].insert(id);
            positions.push_back(peek.tag);
        }

        sub.nullable = false;
        sub.firstpos.insert(id);
        sub.lastpos.insert(id);

        break;
    }

    peek = lex.next();
    if (debug && peek.tag == FINISH_CHAR) {
        fprintf (stderr, "Peek end\n");
    }

    if (debug) fprintf (stderr, "Leave F\n");
    return sub;
}

RegNode RegParser::node_or(const RegNode& left, const RegNode& right)
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

RegNode RegParser::node_cat(const RegNode& left, const RegNode& right)
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
    if (right.nullable)
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

RegNode RegParser::node_closure(const RegNode& node)
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

void RegParser::print() const
{
        printf("----------------------------\n");
        printf("|  Char  |      Positions  |\n");
        printf("----------------------------\n");
    for (postable_t::const_iterator it = symbol_follows.begin();
         it != symbol_follows.end(); ++it)
    {
        char buffer[32]; int k = 0;
        for (pos_t::const_iterator j = it->second.begin();
             j != it->second.end(); ++j)
        {
            sprintf (buffer + k * 2, "%d ", *j);
            ++k;
        }
        printf("|    %c   | %15s |\n", (char)(it->first > 0 ? it->first : '$'), buffer);
    }
        printf("----------------------------\n\n");

        printf("----------------------------\n");
        printf("|   Pos  |        Follows  |\n");
        printf("----------------------------\n");
    for (postable_t::const_iterator it = followpos.begin();
         it != followpos.end(); ++it)
    {
        char buffer[32] = { 0 }; int k = 0;
        for (pos_t::const_iterator j = it->second.begin();
             j != it->second.end(); ++j)
        {
            sprintf (buffer + k * 2, "%d ", *j);
            ++k;
        }
        printf("|   %3d  | %15s |\n", it->first, buffer);
    }
        printf("----------------------------\n\n");

        printf("----------------------------\n");
        printf("|  State |      Positions  |\n");
        printf("----------------------------\n");
    for (vector<DState>::const_iterator it = graph.DStates.begin();
         it != graph.DStates.end(); ++it)
    {
        char buffer[32] = { 0 }; int k = 0;
        for (pos_t::const_iterator j = it->positions.begin();
             j != it->positions.end(); ++j)
        {
            sprintf (buffer + k * 2, "%d ", *j);
            ++k;
        }
        printf("|   %3d  | %15s |\n", it->id, buffer);
    }
        printf("----------------------------\n\n");

        printf("----------------------------\n");
        printf("|  State |          Dtran  |\n");
        printf("----------------------------\n");
    for (DFAGraph::M3D_t::const_iterator it = graph.Dtran.begin();
         it != graph.Dtran.end(); ++it)
    {
        char buffer[32] = { 0 }; int k = 0;
        for (DFAGraph::M2D_t::const_iterator j = it->second.begin();
             j != it->second.end(); ++j)
        {
            sprintf (buffer + k * 4, "%c:%d ", (char)j->first, j->second);
            ++k;
        }
        printf("|   %3d  | %15s |\n", it->first, buffer);
    }
        printf("----------------------------\n\n");
}

} /* lexi */
