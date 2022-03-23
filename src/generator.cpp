/*
 *  Main authors:
 *     Stefano Gualandi <stefano.gualandi@gmail.com>
 *
 */

#include <iostream>
#include <fstream>
#include <sstream>

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
  /// it takes 5 input integers:
  ///  - n => number of vertices
  ///  - d => density of the graph
  ///  - s => seed number
  ///  - t => type of instance (DIMACS, AMPL)

  if ( argc != 5 ) {
    cout << "\n\t usage: generator <n> <d> <s> <t>\n\n"
	 << "where:\n"
	 << "\t - n => size of the first shore\n"
	 << "\t - d => (float) density of the bipartite graph\n"
	 << "\t - s => seed number\n"
	 << "\t - t => type: 0 -> DIMACS, 1 -> AMPL, 2 -> DIMACS binary\n\n";
    exit(-1);
  }
  
  int     n = atoi(argv[1]);
  double  d = 1.0 - atof(argv[2]);
  int     s = atoi(argv[3]);
  int     t = atoi(argv[4]);

  // This is a typedef for a random number generator.
  // Try boost::mt19937 or boost::ecuyer1988 instead of boost::minstd_rand
  typedef boost::minstd_rand base_generator_type;

  /// Define a random number generator and initialize it with a reproducible  seed.
  base_generator_type generator(42u);

  /// Define a uniform random number distribution which produces "double"
  /// values between 0 and 1 (0 inclusive, 1 exclusive).
  boost::uniform_real<> uni_dist(0,1);
  boost::variate_generator<base_generator_type&, boost::uniform_real<> > uni(generator, uni_dist);
  boost::uniform_int<> uni_dist_int(1,10);
  boost::variate_generator<base_generator_type&, boost::uniform_int<> > Uniform1N(generator, uni_dist_int);
  generator.seed(static_cast<unsigned int>(s));
  
  Graph g(n);
  for ( int i = 0; i < n; i++ )
    for ( int j = i+1; j < n; j++ )
      if ( uni() >= d )
	add_edge(i, j, g);
  
  /// Write data in DIMACS format
  if ( t == 0 ) {
    /// Write the data file in std::out stream
    cout << "c graph gen seed " << s << endl;
    cout << "p col " << num_vertices(g) << " " << num_edges(g) << endl;
    
    /// Write the matrix d with given density,
    /// that is, an edge (i,j) of the bipartite graph is present with probability <d>
    forall ( Edge e, edges(g) )
      cout << "e " << 1+source(e,g) << " " << 1+target(e,g) << endl;
  }

  /// Write graph in AMPL format
  if ( t == 1 ) {
    /// Write the data file in std::out stream
    cout << "data;" << endl;
    cout << "param n := " << n << ";" << endl;
    
    /// Da completare
  }

  /// Write graph in binary DIMACS format
  if ( t == 2 ) {
    std::ostringstream name;
    name << "g-" << n << "-" << (1.0-d) << "-" << s << ".b";
    graph_write_dimacs_binary_file ( g, name.str().c_str() );
  }

  return EXIT_SUCCESS;
}
