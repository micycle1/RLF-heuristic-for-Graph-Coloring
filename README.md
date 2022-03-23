# RLF-heuristic-for-Graph-Coloring
C++ Implementations of the RLF (Recursive Largest First) heuristic for Graph Coloring.

By M. Chiarandini, G. Galbiati, S. Gualandi (2011), from
[Efficiency issues in the RLF heuristic for graph coloring](https://imada.sdu.dk/~marco/Publications/Files/MIC2011-ChiGalGua.pdf).

## RLF Heuristics

* rlf: a simple C++ porting of the PL-1 implementation of RLF given in the original paper.
* rlfPlus: a C++ implementation of RLF that uses array-based list to store the adjacent lists of the graph.
* rlfLazy: a C++ implementation of the Lazy RLF algorithm proposed in the paper.

## Utilities

* generator: generate random uniform graph in the binary graph coloring DIMACS format
* converter: convert binary file format in tex file format