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

#include "read_dimacs_bin.hpp"

/// My Graph data structure
typedef unsigned int                  Color;

/// Trace macro
#ifndef DEBUG
#define DEBUG false
#endif
#define TRACE(X)  if( DEBUG ) X;

double DD = 0.0; /// Threshold for using one algorithm instead of the other

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

  inline unsigned int initDegreeToU ( void ) {
    /// Init degree to U of the selected vertex v
    unsigned int du = 0;
    for ( AdjIter u = getIter(); u(); ++u ) {
      du += u.inP();
    }
    return du;
  }
  
  inline unsigned int degreeToUDense ( unsigned int du_max ) {
    if ( d < du_max )
      return 0;

    unsigned int du = d;
    for ( AdjIter u = getIter(); u(); ++u ) {
      du -= u.inP();
      if ( du < du_max )
 	return du;
    }
    return du;
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
  inline unsigned int  degreeToUSparse ( void )  const { return u;                 }
  /// Get the current position of the "cursor"
  inline  AdjNode*     getNextNode     ( void )  const { return &as[d+1];          }
  /// Get the vertex degree
  inline  AdjIter      getIter         ( void )  const { return AdjIter( &as[0] ); }  
  /// Get the vertex degree
  inline  unsigned int degree          ( void )  const { return d;                 }
  /// Decrease vertex degree
  inline  void         reduceDegree    ( void )        { assert(d > 0); d--;       }
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
	  m++;
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
  
  ~Graph() {     
    delete[] vs; 
  }
  /// Decrease vertex degree
  inline  void         reduceVertices ( void )           { assert(n >= 0); n--;       }
  /// Decrease vertex degree
  inline  void         reduceEdges    ( unsigned int d ) { assert(m >= 0); m -= d;       }
protected:
  unsigned int  n;   /// Number of vertices in the list-graph
  unsigned int  m;   /// Number of edges in the list-graph
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

  Vertex* selectVertexDense() {
    /// Select first vertex from graph V
    Vertex* v = maxDegree();

    unsigned int du_max = v->initDegreeToU();

    for ( VertexIter w(P); w(); ++w ) {
      /// Note that du cannot be greater than du_max if (degree(w,G) < du_max)
      Vertex* pw = w.vertex();
      
      /// Update the degree to U and to V
      unsigned int du = pw->degreeToUDense( du_max );
      
      /// Select vertex with maximum degree induced by U, break ties...
      if ( du > du_max || (du == du_max && pw->degree() < v->degree()) ) {
	/// Update new information
	du_max = du;
	v      = pw;
      }
    }
    
    return v;
  }

  Vertex* selectVertexSparse() {
    VertexIter w(P);
    Vertex* v = w.vertex();
    unsigned int du_max = v->degreeToUSparse();
    ++w;
    for ( ; w(); ++w ) {
      Vertex* pw = w.vertex();
      unsigned int du = pw->degreeToUSparse();
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
  void moveNeighborsDense ( Vertex* v ) {
    for ( AdjIter w = v->getIter(); w(); ++w ) {
      if ( w.inP() ) {
	Vertex* pw = w.node();
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
    reduceVertices();            /// Reduces by 1 the number of vertices
    reduceEdges( v->degree() );  /// Reduces by d the number of edges
    /// Remove vertex v from G
    v->clear_vertex();
    /// Remove v from P
    v->skip();
  }

  /// Move delta(v) from V to U
  void moveNeighborsSparse ( Vertex* v ) {
    for ( AdjIter w = v->getIter(); w(); ++w ) {
      if ( w.inP() ) {
	Vertex* pw = w.node();
	/// DIFFERENT: Update degree for all neighbors of node w in P different from v
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
    reduceVertices();            /// Reduces by 1 the number of vertices
    reduceEdges( v->degree() );  /// Reduces by d the number of edges
    /// Remove vertex v from G
    v->clear_vertex();
    /// Remove v from P
    v->skip();
  }
  
  /// Swap the list U and P
  void swapDense() {
    /// Swap P and U
    P->suc = U->suc;
    if ( P->suc != NULL )
      P->suc->pre = P;
    U->suc = NULL;
    U->pre = U;
    /// Reset the 'inP' values
    for ( VertexIter w(P); w(); ++w )
      w.vertex()->inP = true;
  }
  
  /// Swap the list U and P
  void swapSparse() {
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
  /// Get the vertex degree
  inline  unsigned int num_vertices    ( void )  const { return n;                 }
  inline  unsigned int num_edges       ( void )  const { return m;                 }

private:
  Vertex*  P;
  Vertex*  U;
};

/// Color an independent set with 'color'
unsigned int
new_color_class_dense ( HashedList&    H, 
			Color          color
			) 
{
  /// Select the first vertex
  Vertex* v = H.maxDegree();
  /// Color the selected vertex 
  v->setColor( color );
  /// Move delta(v) from V to U
  H.moveNeighborsDense ( v );
  
  /// Size of the stable set
  unsigned int size = 1;  
  while ( !H.empty() ) {
    /// Select an uncolored vertex from G
    Vertex* v = H.selectVertexDense();
    
    /// Color the selected vertex 
    v->setColor( color );
    
    /// Move delta(v) from V to U; remove v from G and P
    H.moveNeighborsDense ( v );

    /// Size of the stable set
    size++;
  }
  
  /// Swap the set V and U
  H.swapDense();

  return size;
}

/// Color an independent set with 'color'
unsigned int
new_color_class_sparse ( HashedList&    H, 
			 Color          color
			 ) 
{
  /// Select the first vertex
  Vertex* v = H.maxDegree();
  /// Color the selected vertex 
  v->setColor( color );
  /// Move delta(v) from V to U
  H.moveNeighborsSparse ( v );
  
  /// Size of the stable set
  unsigned int size = 1;  
  while ( !H.empty() ) {
    /// Select an uncolored vertex from G
    Vertex* v = H.selectVertexSparse();
    
    /// Color the selected vertex 
    v->setColor( color );
    
    /// Move delta(v) from V to U; remove v from G and P
    H.moveNeighborsSparse ( v );

    /// Size of the stable set
    size++;
  }
  
  /// Swap the set V and U
  H.swapSparse();

  return size;
}

unsigned int
RLF( MyGraph& g ) {
  /// HashList container (abstraction for operation from V to U)
  HashedList H ( g );
  
  /// Init phase
  Color c = 0;
  unsigned int alpha = 0;
  unsigned int l = g.size();

  do {
    double n = double(H.num_vertices());
    double m = double(H.num_edges());
    double d  = m/(n*(n-1)/2.0);
    c++; /// Open new class of color
    if ( d >= DD ) {
      alpha = new_color_class_dense  ( H, c );
    } else {
      alpha = new_color_class_sparse ( H, c );
    }
    l -= alpha;
  } while ( l > 0 );
  
  return c;
}

///------------------------------------------------------------------------------------------
/// UNIT TEST FOR RLF
///------------------------------------------------------------------------------------------

#include <sys/time.h>
#include <sys/resource.h>

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

  if ( argc > 3 ) 
    DD = atof(argv[3]);

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
