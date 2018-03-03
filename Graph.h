#ifndef __GRAPH__H__
#define __GRAPH__H__

#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <map>
#include <set>
#include "puntos.h"

template< class T, class C >
class Graph
{
public:
  struct AuxDijkstra
  {
	  unsigned long vertex;
	  unsigned long parent;
	  float cost;
	  bool operator < ( const AuxDijkstra& b )const{
		return(this->cost < b.cost);
	}
  };
public:

  typedef std::vector< T >                TVertices;
  typedef std::map< unsigned long, C >    TRow;
  typedef std::map< unsigned long, TRow > TMatrix;
  typedef std::vector< bool >             TMarks;
  typedef std::multiset< AuxDijkstra > DijkstraQueue;
  typedef std::vector< unsigned long > TTree;

public:
  Graph( );
  virtual ~Graph( );

  unsigned long AddVertex( const T& v );
  void SetArc( unsigned long i, unsigned long j, const C& c );
  unsigned long BuscarVertice(const T& v);
  unsigned long GetNumberOfVertices( ) const;
  const T& GetVertex( unsigned long i ) const;
  float costo(unsigned long i, unsigned long j);
  bool HasArc( unsigned long i, unsigned long j ) const;
  const C& GetCost( unsigned long i, unsigned long j ) const;
  float CalcularDis(unsigned int a,unsigned int b);
  void PrintPlaneGraph( ) const;
  TVertices PrintPreorderGraph( unsigned long i ) const;
  void PrintLevelsGraph( unsigned long i ) const;
  void PrintGraphAsPNG( const std::string& filename ) const;
  Graph< T, C > operator*( const Graph< T, C >& other );
  bool IsBicoloreable() const;
  TTree Dijkstra( unsigned int start ) const;
  std::vector< unsigned long > DijkstraPath( unsigned long start, unsigned long end) const;

  void PrintGrafo( unsigned long i ) const;


protected:
  void PrintPreorderGraph_Dummy( unsigned long i, TMarks& m, TVertices& v ) const;

public:
  TVertices m_Vertices;
  TMatrix   m_Costs;
};

#include "Graph.hxx"

#endif // __GRAPH__H__

// eof - Graph.h

