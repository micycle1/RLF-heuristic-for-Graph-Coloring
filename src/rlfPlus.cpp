#include <iostream>
using std::cout;
using std::endl;
using std::cerr;
using std::pair;
using std::make_pair;

#include <fstream>
using std::ifstream;

#include <cassert>

#include <numeric>
using std::accumulate;

#include "read_dimacs_bin.hpp"

/// For short integers
#include <stdint.h>
#include <sys/time.h>
#include <sys/resource.h>

#ifndef _MY_GRAPH_
#define _MY_GRAPH_
typedef vector<bool>           AdjacencyList;
typedef vector<AdjacencyList>  MyGraph;
#endif


/// My Graph data structure
typedef unsigned int                  Color;

/// Trace macro
#ifndef DEBUG
#define DEBUG false
#endif
#define TRACE(X)  if( DEBUG ) X;

class Vertex;

///------------------------------------------------------------------------------------------
/// My array based list
class AdjNode {
public:
  AdjNode ( void ) : node(NULL), pos(NULL), suc(NULL), pre(NULL) {}

  Vertex*  node;      /// Adjacent vertex (needed to decrease its vertex degree)
  AdjNode* pos;       /// Position of the copy of this node in the list of the other vertex (needed to skip)
  AdjNode* suc;       /// index of successor node
  AdjNode* pre;       /// index of predecessor node

  /// "Skip" this vertex from the list/
  /// Do not remove from memory, simple skip it from the list of successors
  inline
  void skip( void ) {
    /// Update the successors and predecessors pointers
    pre->suc = suc;
    if ( suc != NULL )
      suc->pre = pre;
  }
};

///--------------------------------------------------
/// Adjacency List Iterator
class AdjIter {
public:
  bool operator()() const { return (v != NULL); }
  void operator++()       { v = v->suc; }
  
  inline Vertex*   node()  const   { assert( v != NULL ); return v->node; }
  inline AdjNode*  pos()   const   { assert( v != NULL ); return v->pos;  }
  
  void updateU ( void );
  bool inP     ( void ) const;

private:
  AdjIter ( AdjNode* v0 ) : v(v0->suc) {}
  
  AdjNode* v;
  
  friend class Vertex;
};

///--------------------------------------------------
/// Adjacency List
class Vertex {
private:
  unsigned int  d;       /// Degree of the vertex
  unsigned int  c;       /// Color of the vertex
  AdjNode*      as;      /// Adjacency list

public:
  unsigned int  u;       /// Degree of the vertex induced by U
  bool          inP;     /// If this vertex is still in the vertex set P (potential vertices)
  Vertex*       suc;     /// Successor vertex in the list
  Vertex*       pre;     /// Predecessor vertex in the list

  explicit Vertex ( void ) : d(0), as(NULL), u(0), inP(true), suc(NULL), pre(NULL) {}

  void setAdjList ( unsigned int d0 ) {
    as = new AdjNode[d0+1];
  }
  
  /// Destructor
  ~Vertex() { if ( as != NULL ) delete[] as; }

  /// "Skip" this vertex from the list/
  /// Do not remove from memory, simple skip it from the list of successors
  void skip( void ) {
    /// Update the successors and predecessors pointers
    pre->suc = suc;
    if ( suc != NULL )
      suc->pre = pre;
  }

  /// Insert an element in the back of the list
  void insert ( Vertex* v, AdjNode* w ) {
    /// Increment the "cursor"
    d++;
    /// Add the new "edge"
    as[d].node  = v;
    as[d].pos   = w;
    as[d].pre   = &as[d-1];
    as[d-1].suc = &as[d];
  }

  

  /// Remove every edge incident to vertex "v" from \delta(v).
  /// There is no need to clear also vertex "v", since it will not visited again
  inline void clear_vertex ( void ) {
    for ( AdjIter w = getIter(); w(); ++w ) {
      w.node()->reduceDegree();   /// Reduce the degree of the opposite vertex
      w.pos()->skip();            /// Skip the node from the P list
    }
  }
  /// Get the degree induced by U
  inline unsigned int  degreeToU    ( void )  const { return u;                 }
  /// Get the current position of the "cursor"
  inline  AdjNode*     getNextNode  ( void )  const { return &as[d+1];          }
  /// Get the vertex degree
  inline  AdjIter      getIter      ( void )  const { return AdjIter( &as[0] ); }  
  /// Get the vertex degree
  inline  unsigned int degree       ( void )  const { return d;                 }
  /// Decrease vertex degree
  inline  void         reduceDegree ( void )        { assert(d > 0); d--;       }
  /// Decrease vertex degree
  inline  void         setColor ( unsigned int c0 ) { c = c0; }
};

/// Adjacency List Iterator
class VertexIter {
public:
  VertexIter ( Vertex* ls0 ) : ls(ls0->suc) {}
    
  inline bool operator()() const { return (ls != NULL); }
  inline void operator++()       { ls = ls->suc; }
  
  inline Vertex*      vertex() const { assert( ls != NULL ); return ls;           }
  inline bool         inP()    const { assert( ls != NULL ); return ls->inP;      }
private:
  Vertex* ls;
};


inline bool AdjIter::inP( void )   const   { assert( v != NULL ); return v->node->inP;  }
inline void AdjIter::updateU( void )       { assert( v != NULL ); v->node->u++; }

///------------------------------------------------------------------------------------------
class Graph {
public:
  Graph ( const MyGraph& G0 ) : n(G0.size()) {
    /// Initialize the set of vertices
    vs = new Vertex[n];
    for ( unsigned int v = 0; v < n; v++ ) {
      int d = accumulate( G0[v].begin(), G0[v].end(), 0 );
      vs[v].setAdjList( d ); 
      if ( v > 0 ) {
	vs[v].pre   = &vs[v-1];
	vs[v-1].suc = &vs[v];
      }
    }
    
    /// Initialize all the adjacency lists of the vertices
    /// The input graph need to have index over the vertices 
    for ( unsigned int v = 0; v < n; v++ ) {
      for ( unsigned int w = v+1; w < n; w++ ) {
	if ( G0[v][w] ) {
	  Vertex* i = &vs[v];
	  Vertex* j = &vs[w];
	  /// Get the next free position in the adjacency list
	  AdjNode* p_i = i->getNextNode();
	  AdjNode* p_j = j->getNextNode();
	  /// Insert the edge in the two vertex adj. lists
	  i->insert( j, p_j );
	  j->insert( i, p_i );
	}
      }
    }
  }
  
  ~Graph() { delete[] vs; }
protected:
  unsigned int  n;   /// Number of vertices in the list
  Vertex*       vs;  /// Vertex list
};


/// Data structure for handling constant time operations
/// on "tow-staged monotone" containers:
/// first they just grow, then they just shrink
class HashedList : public Graph {
public:
  explicit HashedList( const MyGraph& G0 ) 
    : Graph(G0) {
    /// Initialiaze pointer P and U
    P = new Vertex();
    U = new Vertex();
    /// Init P list
    P->suc = &vs[0];
    vs[0].pre = P;
    U->pre = U;
  }

  ~HashedList() { 
    delete P;
    delete U;
  }

  Vertex* selectVertex() {
    VertexIter w(P);
    Vertex* v = w.vertex();
    unsigned int du_max = v->degreeToU();
    ++w;
    for ( ; w(); ++w ) {
      Vertex* pw = w.vertex();
      unsigned int du = pw->degreeToU();
      if ( ( du > du_max ) ||
	   ( du == du_max && pw->degree() < v->degree() ) ) {
	/// Update new information
	du_max = du;
	v      = pw;
      }
    }
    return v;
  }

  /// Find the node with maximum degree
  Vertex* maxDegree( void ) {
    VertexIter w(P);
    Vertex* v = w.vertex();
    ++w;
    for ( ; w(); ++w ) {
      Vertex* pw = w.vertex();
      if ( ( pw->degree() > v->degree() ) ||
	   ( pw->degree() == v->degree() && rand()%2 ) )
	v = pw;
    }
    return v;
  }

  /// Move delta(v) from V to U
  void moveNeighbors ( Vertex* v ) {
    for ( AdjIter w = v->getIter(); w(); ++w ) {
      if ( w.inP() ) {
	Vertex* pw = w.node();
	/// Update degree for all neighbors of node w in P different from v
	for ( AdjIter u = pw->getIter(); u(); ++u ) 
	  u.updateU();

	/// Remove from P
	pw->skip();
	/// Add node pw in back to U
	pw->suc = NULL;
	pw->pre = U->pre;
	U->pre->suc = pw;
	U->pre  = pw;  
	/// Wheter the vertex is in P
	pw->inP = false;
      }
    }
    /// Remove vertex v from G
    v->clear_vertex();
    /// Remove v from P
    v->skip();
  }
  
  /// Swap the list U and P
  void swap() {
    /// Swap P and U
    P->suc = U->suc;
    if ( P->suc != NULL )
      P->suc->pre = P;
    U->suc = NULL;
    U->pre = U;
    /// Reset the 'inP' values
    for ( VertexIter w(P); w(); ++w ) {
      w.vertex()->inP = true;
      w.vertex()->u   = 0; /// Serve???
    }
  }
  
  /// Return true if the list V is empty
  bool empty() const { return (P->suc == NULL); }
  
private:
  Vertex*  P;
  Vertex*  U;
};

/// Color an independent set with 'color'
unsigned int
new_color_class ( HashedList&    H, 
		  Color          color
		  ) 
{
  /// Select the first vertex
  Vertex* v = H.maxDegree();
  /// Color the selected vertex 
  v->setColor( color );
  /// Move delta(v) from V to U
  H.moveNeighbors ( v );
  
  /// Size of the stable set
  unsigned int size = 1;  
  while ( !H.empty() ) {
    /// Select an uncolored vertex from G
    Vertex* v = H.selectVertex();
    
    /// Color the selected vertex 
    v->setColor( color );
    
    /// Move delta(v) from V to U; remove v from G and P
    H.moveNeighbors ( v );

    /// Size of the stable set
    size++;
  }
  
  /// Swap the set V and U
  H.swap();

  return size;
}

unsigned int
RLF( MyGraph& g ) {
  /// HashList container (abstraction for operation from V to U)
  HashedList H ( g );

  /// Init phase
  Color c = 0;
  unsigned int alpha = 0;
  unsigned int n = g.size();

  do {
    c++; /// Open new class of color
    alpha = new_color_class ( H, c );
    n -= alpha;
  } while ( n > 0 );
  
  return c;
}

///------------------------------------------------------------------------------------------
/// UNIT TEST FOR RLF
///------------------------------------------------------------------------------------------

int
main(int argc, char* argv[])
{
  /// Input file
  if (2 > argc) {
    cout << "Must specify a filename!\n";
    return -1;
  }

  if ( argc > 2 )
    srand(atoi(argv[2]));
  
  ifstream infile(argv[1]); 
  if (! infile) 
    { 
      cerr << "No " << argv[1] << " file" << endl; 
      exit ( EXIT_FAILURE ); 
    }

  /// Beautify output
  cout.setf(std::ios_base::fixed, std::ios_base::floatfield);
  cout.precision(3);

  MyGraph g;  
  read_dimacs_bin ( g, argv[1] );
  infile.close();

  if ( g.size() > UINT16_MAX ) {
    printf ("The input graph has more than %u vertices. That's too much :P\n", UINT16_MAX);
    exit(1);
  }
  
  struct rusage tempo;
  long int prg_sec0,prg_microsec0,sys_sec0,sys_microsec0;
  long int prg_sec,prg_microsec,sys_sec,sys_microsec;

  getrusage(RUSAGE_SELF,&tempo);
  prg_sec0=tempo.ru_utime.tv_sec;  prg_microsec0=tempo.ru_utime.tv_usec;
  sys_sec0=tempo.ru_stime.tv_sec;  sys_microsec0=tempo.ru_stime.tv_usec;

  int xhi = RLF(g);
  cout << "X(G): " << xhi;

  getrusage(RUSAGE_SELF,&tempo);
  prg_sec= tempo.ru_utime.tv_sec-prg_sec0;
  sys_sec= tempo.ru_stime.tv_sec-sys_sec0;
  prg_microsec=tempo.ru_utime.tv_usec-prg_microsec0;
  sys_microsec=tempo.ru_stime.tv_usec-sys_microsec0;
  printf("\tCPU: %5.3f sec   Sys: %5.3f sec\n",
	 prg_sec+(prg_microsec/1E6),sys_sec+(sys_microsec/1E6));

  return 1;
}
