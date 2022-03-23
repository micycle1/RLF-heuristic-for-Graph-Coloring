#include <iostream>
#include <cstring>

#include <fstream>
using std::ifstream;

using namespace std;
#include "read_dimacs_bin.hpp"

#include <cassert>

#define ADDRESS(I) ((I>>3)+1)*((I>>3)*4+(I&7L))

// dimacs_get_params() reads DIMACS header to set
// the number of vertices and number of edges
void dimacs_get_params(char* preamble, int& n, int& m) {
  char c;
  char *pp = preamble;
  int stop = 0;
  char *tmp = new char[64];
  
  n = m = 0;
  
  while (!stop && (c = *pp++) != '\0'){
    switch (c)
      {
      case 'c':
        while ((c = *pp++) != '\n' && c != '\0');
        break;

      case 'p':
        sscanf(pp, "%s %d %d\n", tmp, &n, &m);
        stop = 1;
        break;

      default:
        break;
      }
  }

  delete[] tmp;
}

// dimacs_get_edge() checks is there edge (i,j) (i>j) in a DIMACS bitmap.
// Returns true if yes and false otherwise
inline bool dimacs_get_edge(const unsigned char* bitmap, int i, int j) {
  return bitmap[ADDRESS(i) + (j>>3)] & (1U << (7-(j&7)));
}

// Graph loading constructor:
// name -- a DIMACS graph file name
// weights_name -- a name of file containing a list of vertex weights
//    (all weights will be 1 if NULL)
// complement -- a flag pointing is an original or the complementary graph needed
void read_dimacs_bin ( MyGraph& g, const char* name ) {
  FILE* graph_file = fopen(name,"rb");

  if ( graph_file ==NULL ) {
    printf("ERROR: Cannot open infile %s\n", name);
    exit(EXIT_FAILURE);
  }
  
  int pr_len;
  if ( !fscanf ( graph_file, "%d\n", &pr_len) ) {
    printf("ERROR: Corrupted preamble %s\n", name);
    exit(EXIT_FAILURE);
  }
  
  
  int n;  /// number of vertices
  int m;  /// number of edges
  char * header = new char[pr_len+1]; /// a text describing the graph

  /// risolvi il warning
  fread ( header, 1, pr_len, graph_file );
  header[pr_len] = '\0';
  dimacs_get_params(header, n, m);
  if ( n == 0 ) {
    printf("ERROR: Corrupted preamble %s\n", name);
    exit(-1);
  }
  delete[] header;
  
  int bmp_size = ADDRESS(n);
  unsigned char* bitmap = new unsigned char[bmp_size];
  
  fread ( bitmap, 1, bmp_size, graph_file );
  fclose ( graph_file );
  
  for ( int i = 0; i < n; i++ ) {
    vector<bool> adj(n, false);
    g.push_back( adj );
  }
  
  for ( int i = 1; i < n; i++ ) 
    for (  int j = 0; j < i; j++ )
      if ( dimacs_get_edge(bitmap,i,j) )
	g[j][i] = g[i][j] = true;
  
  assert ( int(g.size()) == n );

  delete[] bitmap;
}

#ifdef TEST_read_dimacs_bin

int main (int argc, char* argv[]) 
{
  MyGraph g;  

  read_dimacs_bin ( g, argv[1] );

  for ( unsigned int i = 0; i < g.size(); i++ ) {
    cout << i << ": ";
    for ( unsigned int j = 0; j < g[i].size(); j++ )
      if ( g[i][j] || g[j][i])
	cout << "(" << i << ", " << j << ")\t";
  }

  return EXIT_SUCCESS;
}

#endif
