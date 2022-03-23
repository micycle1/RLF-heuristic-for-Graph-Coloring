/*
 *  Main authors:
 *     Stefano Gualandi <stefano.gualandi@gmail.com>
 *
 */

#include <iostream>
#include <fstream>

#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>

/// Boost Graph Library
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
using namespace boost;
typedef adjacency_list < vecS, vecS, undirectedS >   Graph;
typedef graph_traits<Graph>::edge_descriptor    Edge;

/// Forall define from Boost
#include <boost/foreach.hpp>
#define forall         BOOST_FOREACH

#include<boost/tokenizer.hpp>

using namespace std;

/*
 * graph_write_dimacs_binary()
 *
 * Writes a binary dimacs-format file of graph g, with comment, to the
 * file stream fp.
 *
 * Returns TRUE if successful, FALSE if an error occurred.
 */
typedef unsigned long int setelement;
#define SET_BIT_MASK(x) ((setelement)1<<(x))

#define STR_APPEND(s)			     \
  if (headerlength+strlen(s) >= headersize) {	\
    headersize+=1024;				\
    header=(char*)realloc(header,headersize);	\
  }						\
  strncat(header,s,1000);			\
  headerlength+=strlen(s);

bool graph_write_dimacs_binary ( const Graph& g, FILE *fp ) {
  char *buf;
  char *header=NULL;
  unsigned int headersize=0;
  unsigned int headerlength=0;
  int n = num_vertices(g);

//   ASSERT((sizeof(setelement)*8)==ELEMENTSIZE);
//   ASSERT(graph_test(g,NULL));
//   ASSERT(fp!=NULL);

  buf=(char*)malloc(std::max(1024,n/8+1));
  header=(char*)malloc(1024);
  header[0]=0;
  headersize=1024;

  sprintf(buf,"p edge %d %d\n",n,int(num_edges(g)));
  STR_APPEND(buf);
  
  fprintf(fp,"%d\n",(int)strlen(header));
  fprintf(fp,"%s",header);
  free(header);

  for ( int i = 0; i < n; i++) {
    memset(buf,0,i/8+1);
    for ( int j = 0; j<i; j++)
      if ( edge(i,j,g).second )
	buf[j/8] |= SET_BIT_MASK(7-j%8);

    fwrite(buf,1,i/8+1,fp);
  }
  free(buf);
  
  return true;
}

/*
 * graph_write_dimacs_binary_file()
 *
 * Writes a binary dimacs-format file of graph g, with comment, to
 * given file.
 *
 * Returns TRUE if successful, FALSE if an error occurred.
 */
bool graph_write_dimacs_binary_file ( const Graph& g, const char *file) {
  FILE *fp;
  
//   ASSERT((sizeof(setelement)*8)==ELEMENTSIZE);
//   ASSERT(file!=NULL);
  
  if ((fp=fopen(file,"wb"))==NULL)
    return false;

  if (!graph_write_dimacs_binary(g,fp)) {
    fclose(fp);
    return false;
  }

  fclose(fp);
  return true;
}


/// ------------------------------------------------------------------------------------------
int main (int argc, char* argv[]) 
{
  string filename = argv[1];

  /// Read instance from the OR-lib
  std::ifstream infile(filename.c_str()); 
  if (!infile) 
    exit ( EXIT_FAILURE ); 

  int n;     /// Number of variables
  int m;     /// Number of constraints
  int k;     /// 

  infile >> n >> m >> k;
  string buf;
  std::getline(infile, buf);

  Graph g;
  for ( int i = 0; i < n; i++ )
    add_vertex(g);
  
  for ( int i = 0; i < n; i++ ) {
    string buf;
    std::getline(infile, buf);
    
    tokenizer<> tok(buf);
    for ( tokenizer<>::iterator beg=tok.begin(); beg!=tok.end(); ++beg)
      if ( i < atoi((*beg).c_str())-1 )
	add_edge( i, atoi((*beg).c_str())-1, g );
  }
  
  filename.append(".b");
  graph_write_dimacs_binary_file ( g, filename.c_str() );
  
  /// Write data in DIMACS format
//   cout << "c graph converted " << argv[1] << endl;
//   cout << "p col " << num_vertices(g) << " " << num_edges(g) << endl;
  
//   /// Write the matrix d with given density,
//   /// that is, an edge (i,j) of the bipartite graph is present with probability <d>
//   forall ( Edge e, edges(g) )
//     cout << "e " << 1+source(e,g) << " " << 1+target(e,g) << endl;

//   /// Write graph in AMPL format
//   if ( t == 1 ) {
//     /// Write the data file in std::out stream
//     cout << "data;" << endl;
//     cout << "param n := " << n << ";" << endl;
    
//     /// Da completare
//   }

  return EXIT_SUCCESS;
}
