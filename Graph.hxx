
#ifndef __GRAPH__HXX__
#define __GRAPH__HXX__

#include <cstdlib>
#include <queue>
#include <limits>
#include <sstream>
#include <vector>
#include <algorithm>
#include <deque>

// -------------------------------------------------------------------------
template< class T, class C >
Graph< T, C >::
Graph( )
{
}

// -------------------------------------------------------------------------
template< class T, class C >
Graph< T, C >::
~Graph( )
{
}

// -------------------------------------------------------------------------
template< class T, class C >
unsigned long Graph< T, C >::
AddVertex( const T& v )
{
  this->m_Vertices.push_back( v );
  return( this->m_Vertices.size( ) - 1 );
}

// -------------------------------------------------------------------------
template< class T, class C >
void Graph< T, C >::
SetArc( unsigned long i, unsigned long j, const C& c )
{
  this->m_Costs[ i ][ j ] = c;
}

// -------------------------------------------------------------------------
template< class T, class C >
unsigned long Graph< T, C >::
GetNumberOfVertices( ) const
{
  return( this->m_Vertices.size( ) );
}

// -------------------------------------------------------------------------
template< class T, class C >
const T& Graph< T, C >::
GetVertex( unsigned long i ) const
{
  return( this->m_Vertices[ i ] );
}

// -------------------------------------------------------------------------
template< class T, class C >
bool Graph< T, C >::
HasArc( unsigned long i, unsigned long j ) const
{
  typename TMatrix::const_iterator rIt = this->m_Costs.find( i );
  if( rIt != this->m_Costs.end( ) )
    return( rIt->second.find( j ) != rIt->second.end( ) );
  return( false );
}

// -------------------------------------------------------------------------
template< class T, class C >
const C& Graph< T, C >::
GetCost( unsigned long i, unsigned long j ) const
{
  static const C inf_cost = std::numeric_limits< C >::max( );
  if( this->HasArc( i, j ) )
    return( this->m_Costs[ i ][ j ] );
  else
    return( inf_cost );
}

// -------------------------------------------------------------------------
template< class T, class C >
void Graph< T, C >::
PrintPlaneGraph( ) const
{
  typename TVertices::const_iterator vIt = this->m_Vertices.begin( );
  for( ; vIt != this->m_Vertices.end( ); vIt++ ){
   puntos a=*vIt;
    std::cout <<"  x "<< a.x << " y  "<< a.y<<"  z  "<< a.z;
  std::cout << std::endl;
  }
}

// -------------------------------------------------------------------------
template< class T, class C >
std::vector< T > Graph< T, C >::
PrintPreorderGraph( unsigned long i ) const
{
  TMarks m( this->m_Vertices.size( ), false );
  TVertices v;
  this->PrintPreorderGraph_Dummy( i, m, v );
  return v;
}

// -------------------------------------------------------------------------
template< class T, class C >
void Graph< T, C >::
PrintLevelsGraph( unsigned long i ) const
{
  std::vector< bool > m( this->m_Vertices.size( ), false );
  std::queue< unsigned long > q;
  q.push( i );
  while( !q.empty( ) )
  {
    unsigned long n = q.front( );
    q.pop( );

    if( m[ n ] )
      continue;

    puntos punto=m_Vertices[ n ];
    std::cout <<" x "<< punto.x<<" y "<<punto.y<<"  z "<<punto.z<<std::endl;
    m[ n ] = true;

    typename TMatrix::const_iterator cIt = this->m_Costs.find( n );
    if( cIt != this->m_Costs.end( ) )
    {
      typename TRow::const_iterator rIt = cIt->second.begin( );
      for( ; rIt != cIt->second.end( ); rIt++ )
        q.push( rIt->first );

    } // fi

  } // elihw
  std::cout << std::endl;
}

// -------------------------------------------------------------------------
template< class T, class C >
std::vector< unsigned long > Graph< T, C >::
Dijkstra( unsigned int start ) const
{
	TTree tree( this->m_Vertices.size( ), this->m_Vertices.size( ) );
    std::vector< bool > m( this->m_Vertices.size( ), false );
	DijkstraQueue q;
	AuxDijkstra n;
	n.vertex = start;
  //std::cout<<"n.vertex"<<n.vertex<<std::endl;
	n.parent = start;
	n.cost = 0;
	q.insert(n);
	
	while( !q.empty( ) )
	{
		//std::cout<<q.size()<<std::endl;
		n = *(q.begin());
		q.erase(q.begin());

		if( m[ n.vertex ] ){
			continue;
		}

	   tree[ n.vertex ] = n.parent;
       m[ n.vertex ] = true;

       typename TMatrix::const_iterator cIt = this->m_Costs.find( n.vertex );
       if( cIt != this->m_Costs.end( ) )
       {
          typename TRow::const_iterator rIt = cIt->second.begin( );
          for( ; rIt != cIt->second.end( ); rIt++ )
	  {
			AuxDijkstra nn;
			nn.vertex = rIt->first;
			nn.parent = n.vertex;
			nn.cost = n.cost + rIt->second;
			q.insert( nn );
	  }	

		} // fi

  } // elihw
  return ( tree );

}

// -------------------------------------------------------------------------
template< class T, class C >
typename std::vector< unsigned long > Graph< T, C >::
DijkstraPath( unsigned long start, unsigned long end ) const
{
  TTree tree = this-> Dijkstra ( start );
	std::list< unsigned long > path;
	path.push_front( end );
	unsigned long it = end;
	
	while( tree[ it ] != it )
	{
		path.push_front( it );
		it = tree[ it ];
	}
	path.push_front(start);
	//tenemos implementado dijkstra para lista por lo tanto pasamos la lista a un vector	
	std::list< unsigned long >::iterator cit= path.begin();
	std::vector < unsigned long > path2;
	for(; cit!=path.end(); cit++ )
	{
	  path2.push_back(*cit);
	}	
  /*std::vector< puntos >  vecto;

  for(unsigned long i=0;i<path2.size()-1;i++){
      puntos punto=m_Vertices[ i ];
      vecto.push_back(punto);
      std::cout <<" x "<< punto.x<<" y "<<punto.y<<"  z "<<punto.z<<std::endl;
  }*/
	return( path2 );
}
// -------------------------------------------------------------------------
template< class T, class C >
void Graph< T, C >::
PrintGraphAsPNG( const std::string& filename ) const
{
  std::stringstream str;
  str << "echo \"digraph G{";

  typename TMatrix::const_iterator cIt = this->m_Costs.begin( );
  for( ; cIt != this->m_Costs.end( ); cIt++ )
  {
    typename TRow::const_iterator rIt = cIt->second.begin( );
    for( ; rIt != cIt->second.end( ); rIt++ )
    {
      str << cIt->first << "->" << rIt->first << " ";
      str
        << cIt->first << " [label=\""
        << this->m_Vertices[ cIt->first ] << "\"]; ";
      str
        << rIt->first << " [label=\""
        << this->m_Vertices[ rIt->first ] << "\"]; ";

    } // rof

  } // rof

  str << "}\" | dot -Tpng > " << filename;
  std::system( str.str( ).c_str( ) );
}

// bicoloreable------------------------------------------------------------------------- 
template< class T, class C >
bool Graph< T, C >::
IsBicoloreable( ) const
{
  bool bicolor=true;// 0:color1 ... 1:color:2 .... 2:no_color
  typename TVertices::iterator i = m_Vertices.begin();
  for( i; i!= m_Vertices.end() && bicolor!=false ; i++ )
  {
	  std::vector< unsigned char > m( this->m_Vertices.size( ), 2 );
	  std::queue< unsigned long > q;
	  q.push( i );
	  unsigned char actual_color=0;
	  while( !q.empty( ) && bicolor)
	  {
	    unsigned long n = q.front( );
	    q.pop( );

	    if( m[ n ] = 2)
	    {
	      m[ n ] = actual_color;
	      actual_color = ( actual_color + 1 ) % 2;
	    }

	    typename TMatrix::const_iterator cIt = this->m_Costs.find( n );
	    if( cIt != this->m_Costs.end( ) )
	    {
	      typename TRow::const_iterator rIt = cIt->second.begin( );
	      for( ; rIt != cIt->second.end( ); rIt++ )
		if(m[ rIt->first ] != 2)
		{
		   if(m[ rIt->first ] == m[ n ])
		      bicolor = false;
		}
		else
		 q.push( rIt->first );
	    } 

	  } 
  }
  return( bicolor );
}

// -------------------------------------------------------------------------
template< class T, class C >
Graph< T, C > Graph< T, C >::
operator*( const Graph< T, C >& other )
{

}

// -------------------------------------------------------------------------
template< class T, class C >
void Graph< T, C >::
PrintPreorderGraph_Dummy( unsigned long i, TMarks& m, TVertices& v) const
{
  if( m[ i ] )
    return;

  //std::cout << this->m_Vertices[ i ].x << "- "<< this->m_Vertices[ i ].y <<std::endl;
  v.push_back( this->m_Vertices[ i ] );
  m[ i ] = true;

  typename TMatrix::const_iterator cIt = this->m_Costs.find( i );
  if( cIt != this->m_Costs.end( ) )
  {
    typename TRow::const_iterator rIt = cIt->second.begin( );
    for( ; rIt != cIt->second.end( ); rIt++ )
      this->PrintPreorderGraph_Dummy( rIt->first, m, v );

  } // fi
}



template< class T, class C >
unsigned long Graph< T, C >::
BuscarVertice(const T& v){
unsigned long retorno=0;
   puntos es1, es2;
   es1.x=v.x;
   es1.y=v.y;
   es1.z=v.z;
  for(unsigned long i=0; i<m_Vertices.size();i++){
    es2=m_Vertices[i];
    if(es1.x==es2.x && es1.y==es2.y && es1.z==es2.z){
    // std::cout<<"agaggaggagga"<<es2.x<<"   "<<es2.y<<"  "<<es2.z<<std::endl;
     retorno=i;
    }
  }
  //std::cout<<"retorno"<<m_Vertices[retorno].x<<" "<<m_Vertices[retorno].y<<"    "<<m_Vertices[retorno].z<<std::endl;
  //std::cout<<"id retorno"<<retorno<<std::endl;
 return retorno;
}
template< class T, class C >
float Graph< T, C >:: 
CalcularDis(unsigned int a,unsigned int b){
   puntos ap=m_Vertices[a];
   puntos bp=m_Vertices[b];
   float restax=ap.x - bp.x;
   float restay=ap.y - bp.y;
   float restaz=ap.z - bp.z;
   float suma=pow(restax,2)+pow(restay,2)+pow(restaz,2);
   float retorno =sqrt(suma);
   

return retorno;

}
template< class T, class C >
float Graph< T, C >:: 
costo(unsigned long i, unsigned long j){
   typename TMatrix::const_iterator cIt = this->m_Costs.find( i );
  if( cIt != this->m_Costs.end( ) )
  {
    typename TRow::const_iterator rIt = cIt->second.find(j );
     if( rIt != cIt->second.end( ) ){
            return rIt->second;
      
       }
  }

}
template< class T, class C >
void Graph< T, C >::
PrintGrafo( unsigned long i ) const{
  //TMarks m( this->m_Vertices.size( ), false );
  //for(int a=0; a<m.size(); a++){

  //}

}

#endif // __GRAPH__HXX__

// eof - Graph.hxx
