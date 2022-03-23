//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee
//
// Modified: Stefano Gualandi
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*
  Reads graph coloring problems in DIMACS format.
  This works, but could use some polishing.
*/

/* ----------------------------------------------------------------- */

#include <vector>
#include <iostream>
#include <string>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>


namespace boost {

  template <class Graph>
  int read_dimacs_graph_coloring(Graph& g,
				 std::istream& in)
  {

    //  const int MAXLINE = 100;      /* max line length in the input file */
    const int ARC_FIELDS = 2;     /* no of fields in arc line  */
    const int NODE_FIELDS = 2;    /* no of fields in node line  */
    const int P_FIELDS = 3;       /* no of fields in problem line */
    
    typedef typename graph_traits<Graph>::vertices_size_type  vertices_size_type;
    typedef typename graph_traits<Graph>::vertex_descriptor   Vertex;
    typedef typename graph_traits<Graph>::edge_descriptor     Edge;
    
    std::vector<Vertex> verts;
    
    unsigned int m, n,                    /*  number of edges and nodes */
      i, j, head, tail;
    
    unsigned int no_lines=0,              /* no of current input line */
      no_plines=0,                /* no of problem-lines */
      no_alines=0;                /* no of arc-lines */
    
    std::string in_line;          /* for reading input line */
    char pr_type[5];              /* for reading type of the problem */
    
    int k,                        /* temporary */
      err_no;                     /* no of detected error */
    
    /* -------------- error numbers & error messages ---------------- */
    const int EN1   = 0;
    const int EN2   = 1;
    //  const int EN3   = 2;
    const int EN4   = 3;
    //  const int EN6   = 4;
    //  const int EN10  = 5;
    //  const int EN7   = 6;
    const int EN8   = 7;
    //  const int EN9   = 8;
    const int EN11  = 9;
    const int EN12 = 10;
  //  const int EN13 = 11;
  //  const int EN14 = 12;
    const int EN16 = 13;
    const int EN15 = 14;
    const int EN17 = 15;
    const int EN18 = 16;
    const int EN21 = 17;
    const int EN19 = 18;
    //  const int EN20 = 19;
    const int EN22 = 20;
    
    //   static std::string err_message[] =
    //   {
    //     /* 0*/    "more than one problem line.",
    //     /* 1*/    "wrong number of parameters in the problem line.",
    //     /* 2*/    "it is not a Max Flow problem line.",
    //     /* 3*/    "bad value of a parameter in the problem line.",
    //     /* 4*/    "can't obtain enough memory to solve this problem.",
    //     /* 5*/    "more than one line with the problem name.",
    //     /* 6*/    "can't read problem name.",
    //     /* 7*/    "problem description must be before node description.",
    //     /* 8*/    "this parser doesn't support multiply sources and sinks.",
    //     /* 9*/    "wrong number of parameters in the node line.",
    //     /*10*/    "wrong value of parameters in the node line.",
    //     /*11*/    " ",
    //     /*12*/    "source and sink descriptions must be before arc descriptions.",
    //     /*13*/    "too many arcs in the input.",
    //     /*14*/    "wrong number of parameters in the arc line.",
    //     /*15*/    "wrong value of parameters in the arc line.",
    //     /*16*/    "unknown line type in the input.",
    //     /*17*/    "reading error.",
    //     /*18*/    "not enough arcs in the input.",
    //     /*19*/    "source or sink doesn't have incident arcs.",
    //     /*20*/    "can't read anything from the input file."
    //   };
    /* --------------------------------------------------------------- */
    
    /* The main loop:
       -  reads the line of the input,
       -  analyses its type,
       -  checks correctness of parameters,
       -  puts data to the arrays,
     -  does service functions
    */
    while (std::getline(in, in_line)) {
      ++no_lines;

      if (in_line.size() > 1) /// strange bug on mac: empty line not recognized on .dos file
      switch (in_line[0]) {
      case 'c':                  /* skip lines with comments */
      case '\n':                 /* skip empty lines   */
      case '\0':                 /* skip empty lines at the end of file */
	break;
	
      case 'p':                  /* problem description      */
	if ( no_plines > 0 )
	  /* more than one problem line */
	  { err_no = EN1 ; goto error; }
	
	no_plines = 1;
	
	if (
	    /* reading problem line: type of problem, no of nodes, no of arcs */
	    std::sscanf ( in_line.c_str(), "%*c %s %d %d", pr_type, &n, &m )
	    != P_FIELDS
          )
	  /*wrong number of parameters in the problem line*/
	  { err_no = EN2; goto error; }
	
	if ( n <= 0  || m <= 0 )
	  /*wrong value of no of arcs or nodes*/
	  { err_no = EN4; goto error; }
	
	{
	  for (unsigned int vi = 0; vi < n; ++vi) {
	    verts.push_back( add_vertex(g) );
	    //	    idx[v] = vi;
	  }
	}
	break;
	
      case 'n':                    /* source(s) description */
	if ( no_plines == 0 )
	  /* there was not problem line above */
	  { err_no = EN8; goto error; }
	
	/* reading source  or sink */
	k = std::sscanf ( in_line.c_str(),"%*c %d %d", &i, &j );
	--i; // index from 0
	if ( k < NODE_FIELDS )
	  /* node line is incorrect */
	  { err_no = EN11; goto error; }
	
	if ( i < 0 || i > n )
	  /* wrong value of node */
	  { err_no = EN12; goto error; }
		
	break;
      case 'e':                    /* arc description */
	//       if ( no_nslines == 0 || no_nklines == 0 )
	//         /* there was not source and sink description above */
	//         { err_no = EN14; goto error; }
	
	if ( no_alines >= m )
	  /*too many arcs on input*/
	  { err_no = EN16; goto error; }
	
	if (
	    /* reading an arc description */
	    std::sscanf ( in_line.c_str(),"%*c %d %d",
			  &tail, &head )
	    != ARC_FIELDS
	    )
	  /* arc description is not correct */
	  { err_no = EN15; goto error; }
	
	--tail; // index from 0, not 1
	--head;
	if ( tail < 0  ||  tail > n  ||
	     head < 0  ||  head > n
	     )
	  /* wrong value of nodes */
	  { err_no = EN17; goto error; }
	
	{
	  Edge e1;
	  bool in1;
	  tie(e1, in1) = add_edge(verts[tail], verts[head], g);
	}
	++no_alines;
	break;
	
      default:
	/* unknown type of line */
	err_no = EN18; goto error;
	
      } /* end of switch */
    }     /* end of input loop */
    
    /* ----- all is red  or  error while reading ----- */
    
    if ( in.eof() == 0 ) /* reading error */
      { err_no=EN21; goto error; }
    
    if ( no_lines == 0 ) /* empty input */
      { err_no = EN22; goto error; }
    
    if ( no_alines < m ) /* not enough arcs */
      { err_no = EN19; goto error; }
    
    /* Thanks God! all is done */
    return (0);
    
    /* ---------------------------------- */
  error:  /* error found reading input */
    std::printf("Error in input file\n");
    //   std::printf ( "\nline %d of input - %s\n",
    //                 no_lines, err_message[err_no] );
    
    std::exit (1);
    return (0); /* to avoid warning */
  }
  /* --------------------   end of parser  -------------------*/

} // namespace boost
