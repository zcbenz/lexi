#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>
#include <vector>
#include <set>
#include <string>
#include <map>

namespace lexi {

const int debug = 0;
    
using std::set;
using std::string;
using std::map;
using std::vector;

typedef std::vector<char> buffer_t;
typedef set<int> pos_t;
typedef map<int, pos_t> postable_t;
typedef vector<int> positions_t;

typedef map<int, int> M2D_t;
typedef map<int, M2D_t> M3D_t;

} /* lexi */

#endif /* end of BUFFER_H */
