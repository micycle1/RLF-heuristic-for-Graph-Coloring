#include <iostream>
using std::cout;
using std::endl;
using std::cerr;
using std::pair;
using std::make_pair;

#include <fstream>
using std::ifstream;

#include <vector>
using std::vector;

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

/// Trace macro
#ifndef DEBUG
#define DEBUG false
#endif
#define TRACE(X)  if( DEBUG ) X;

void my_delete ( vector<int16_t>& H, int16_t M, 
		 const vector<int32_t>& CI, 
		 const vector<int16_t>& CL ) {
  /// This subroutine decrements the value of H for M and the nodes adjacent to M
  //  inc_tot++;
  H[M] = -1;
  if ( CI[M] > CI[M-1] ) 
    for ( int32_t P = CI[M-1]+1; P <= CI[M]; P++ )
      H[CL[P]] = H[CL[P]] - 1;
}

int
RLF( const MyGraph& g ) {

  /// Initialize the edge array representation of the graph
  int16_t N  = g.size();
  int16_t N1 = N+1;
  vector<int16_t> C (N1,0);
  vector<int32_t> CI(N1,0);
  vector<int16_t> CL;
  
  {
    int k = 1;
    for ( int i = 0; i < N; i++ ) {
      for ( int j = 0; j < N; j++ )
	if ( g[i][j] ) {
	  CL.push_back( j+1 );
	  k++;
	}
      CI[i+1] = k;
    }
  }

  /// Initialize the color function to zero
  int16_t COL = 0;
  int16_t J   = 0;
  int16_t L   = 1;

  /// Initialize the F vector to the node degrees
  vector<int16_t> F(N1,0);
  vector<int16_t> E(N1,0);
  for ( int16_t I = 1; I < N1; I++ ) 
    F[I] = CI[I] - CI[I-1];
  
  /// If there is any uncolroed nodes, initiate the assignment of the next color
  while ( J < N ) {
    COL++;
    /// Reinitiliaze the E vector
    for ( int16_t I = 1; I < N1; I++ )
      E[I] = F[I];
    /// Select the node in U1 with maximal degree in U1
    for ( int16_t I = 1; I < N1; I++ )
      if ( F[I] > F[L] || (F[I] == F[L] && rand()%2 ) )
	L = I;
    
    /// Color the node just selected and continue to color nodes with
    /// color COL until U1 is empty
    while ( E[L] >= 0 ) {
      /// Color node and modify U1 and U2 accordingly
      my_delete(E, L, CI, CL);
      my_delete(F, L, CI, CL);
      C[L] = COL;
      J++;
      if ( CI[L] > CI[L-1] )
	for ( int32_t I = CI[L-1]+1; I <= CI[L]; I++ )
	  if ( E[CL[I]] >= 0 )
	    my_delete(E, CL[I], CI, CL);
      /// Find the first node in U1, if any
      int16_t K = 0;
      for ( int16_t I = 1; I < N1; I++ )
	if ( E[I] >= 0 ) {
	  K = I;
	  break;
	}
      /// If U1 is not empty, select the next node for coloring
      if ( K > 0 ) {
	L = K;
	for ( int16_t I = K; I < N1; I++ )
	  if ( E[I] >= 0 ) {
	    if ( F[I] - E[I] > F[L] - E[L] )
	      L = I;
	    else {
	      if ( F[I] - E[I] == F[L] - E[L] && E[I] < E[L] ) 
		L = I;
	    }	      
	  }
      }
    }
  }

  return COL;
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

  MyGraph g;
  read_dimacs_bin ( g, argv[1] );
  infile.close();

  if ( g.size() > UINT16_MAX ) {
    printf ("The input graph has more than %u vertices. That's too much :P\n", UINT16_MAX);
    exit(1);
  }
  unsigned int m = 0;
  for ( unsigned int i = 0; i < g.size(); i++ ) {
    m += accumulate( g[i].begin(), g[i].end(), m );
  }
  if ( m > UINT32_MAX ) {
    printf ("The input graph has more than %u edges. That's too much :P\n", INT32_MAX);
    exit(1);
  }

  /// Beautify output
  cout.setf(std::ios_base::fixed, std::ios_base::floatfield);
  cout.precision(3);

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
