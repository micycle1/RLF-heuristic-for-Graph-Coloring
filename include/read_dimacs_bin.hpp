#ifndef _MY_READ_DIMACS_
#define _MY_READ_DIMACS_


#ifndef _MY_GRAPH_
#define _MY_GRAPH_


#include <vector>
using std::vector;

typedef vector<bool>           AdjacencyList;
typedef vector<AdjacencyList>  MyGraph;
#endif

void read_dimacs_bin ( MyGraph& g, const char* name );

#endif
