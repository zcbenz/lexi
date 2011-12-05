#ifndef DFA_H
#define DFA_H

#include <set>
#include <map>

#include "lex.h"

namespace lexi {

using std::set;
using std::map;

typedef set<int> pos_t;
typedef map<int, pos_t> postable_t;
typedef vector<int> positions_t;

struct DState {
    int id;
    pos_t positions;

    DState(int id, const pos_t& positions)
        : id(id), positions(positions)
    {
    }

    bool operator< (const DState& r)
    {
        return id < r.id;
    }

    bool operator== (const DState& r)
    {
        return positions == r.positions;
    }
};

struct DFAGraph {
    typedef map<int, int> M2D_t;
    typedef map<int, M2D_t> M3D_t;

    vector<DState> DStates;
    M3D_t Dtran;
    map<int, buffer_t> actions;

    typedef map<pos_t, int> DStatesMap_t;
    DStatesMap_t DStatesMap;

    DState *push_back(const pos_t& positions);
};

} /* lexi */

#endif /* end of DFA_H */
