#include "dfa.h"

namespace lexi {

DState *DFAGraph::push_back(const pos_t& positions)
{
    DStatesMap_t::const_iterator it = DStatesMap.find(positions);
    if (it != DStatesMap.end()) {
        return &DStates[it->second];
    } else {
        int id = DStates.size();
        DStates.push_back(DState(id, positions));
        DStatesMap.insert(make_pair(positions, id));

        return &DStates.back();
    }
}

} /* lexi */
