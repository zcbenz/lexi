#ifndef DFA_H
#define DFA_H

#include "buffer.h"

namespace lexi {

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
    vector<DState> DStates;
    M3D_t Dtran;
    map<int, buffer_t> actions;

    typedef map<pos_t, int> DStatesMap_t;
    DStatesMap_t DStatesMap;

    DState &push_back(const pos_t& positions);
    void clear() {
        DStates.clear();
    }
};

} /* lexi */

#endif /* end of DFA_H */
