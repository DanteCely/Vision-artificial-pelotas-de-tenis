/**
 * @file ProyectoFinal.cpp
 * @brief Entrega Final Vision Artificial
 * @author Marvin Daniel Cely Báez
 */

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <queue>
#include <string>
#include <math.h>
#include <algorithm>
#include "Graph.h"
#include "puntos.h"
#define BLACK 0
#define WHITE 255
#define PORCENT 15

/*Debe utilizar la información calculada anteriormente para generar una nueva imagen que
retenga sólo las regiones cuyo tamaño sea mayor al 15% del tamaño de la región más
grande.*/

using namespace cv;
typedef Graph<puntos,float> graf;
typedef std::pair< int, int > semilla;
typedef std::queue< semilla > canasta;
std::vector<int> RegionsPixels;
std::vector<semilla> representante;
std::vector<puntos> camino;

///Functions Prototype
void generateImages( Mat imageName );
void writeResult( Mat& img, std::string imageName, std::string resultName );


void tourImage( bool filterReg );
void RegionsFilter( int largestRegion );
int regiones8vecinos( canasta muchasSemillas );
void regiones8vecinosBLACK( canasta muchasSemillas );
void negativeImage( );
int densityColor( int intensity );
void binarizationMatch( );
void showResults( );

void identificarPuntos( Mat img );
Mat generarPerspectiva( Mat& img, double& width, double& heigth );

void segmentacion( Mat& img, int bins );



std::vector<Point2f> findCenters( );
void crearGrafo( semilla posRobot );
void filtroPromedio( Mat& img );
void recorrerDibujarGrafo( );
void MyLine( Mat img, Point start, Point end );


/// Global variables
struct CuatroPuntos{
	int x1, y1;
	int x2, y2;
	int x3, y3;
	int x4, y4;
}fourPoints;
Mat image, gray_image, regionFilter_image, negative_image, drawing;
int TAG_COUNT, DIF = 1;
graf grafo;
std::string nombreImagen;

int main( int argc, char** argv )
{
		/// Get command line arguments
	if( argc != 2 ){
	  	std::cerr << "Usage: " << argv[ 0 ] << " <image>" << std::endl;
	    return( -1 );
	  }
	  image = imread( argv[1] );
	  nombreImagen = argv[1];
	  
	  if( image.empty() ){
	  	std::cerr << "Invalid" << std::endl;
  		return( -1 );
	  }

	double width = 823.0;
	double heigth = 1190.0;
	Mat imagePerspective = Mat::zeros( Size(width,heigth), CV_8UC3 );  
	imagePerspective = generarPerspectiva( image, width, heigth );
	writeResult( imagePerspective, nombreImagen, "_0_perspective.jpg" );

	segmentacion( imagePerspective, 0 );
	writeResult( imagePerspective, nombreImagen, "_2_binarization.jpg" );

	
	generateImages( imagePerspective );

	  ///SE SUAVIZA LA IMAGEN PARA TENER CONTORNOS DEFINIDOS
	  for(int i = 0; i < 5; i++)
		filtroPromedio( gray_image );
	  negativeImage( );//Si la imagen tiene mas pixeles blacon los intercambia
	  ///CARACTERIZAR IMAGEN
	  	regionFilter_image = gray_image;
			 tourImage( true );
			 writeResult( regionFilter_image, nombreImagen , "_3_regionFiltered.jpg" );
		gray_image = regionFilter_image;
			 binarizationMatch( );
			 tourImage( false );
			 writeResult( gray_image, nombreImagen , "_4_characterized.jpg" );
			 showResults();
		semilla posRobot = std::make_pair( gray_image.cols, 0 );//Posicion donde Inicia el robot   gray_image.rows
	    crearGrafo( posRobot );

  
  image.release();
  gray_image.release();
  regionFilter_image.release();
  negative_image.release();
  
return 0;
}

/**
 * @generateImages
 */
void generateImages( Mat imagePerspective ){
  		//cvtColor( imagePerspective, gray_image, CV_BGR2GRAY );
  		//cvtColor( imagePerspective, negative_image, CV_BGR2GRAY );
  		gray_image = imagePerspective;
  		negative_image = imagePerspective;
}

/**
 * @writeResult
 */
void writeResult( Mat& img, std::string imageName, std::string resultName ){
  std::stringstream ss( imageName );
  std::string basename;
  getline( ss, basename, '.' );
  imwrite( basename + resultName, img );
}

/**
 * @negativeImage
 */
void negativeImage( ){

	binarizationMatch( );
	if( densityColor( WHITE ) > densityColor( BLACK ) ){
		for( int i=0; i<gray_image.cols; i++ ){
			for( int j=0; j<gray_image.rows; j++ ){
				uchar pixel = gray_image.at< uchar >(Point(i,j));
				if( (int)pixel == WHITE ){
					negative_image.at< uchar >(Point(i,j)) = BLACK;
				}else{
					negative_image.at< uchar >(Point(i,j)) = WHITE;
				}
			}
		}
	}
	gray_image = negative_image;
}

/**
 * @densityColor
 */
int densityColor( int intensity ){
	int cantidad = 0;
	for( int i=0; i<gray_image.cols; i++ ){
		for( int j=0; j<gray_image.rows; j++ ){
			uchar pixel = gray_image.at< uchar >(Point(i,j));
			if( (int)pixel == intensity ) cantidad++;
		}
	}
	return cantidad;
}

/**
 * @binarizationMatch
 */
void binarizationMatch( ){
	int cantidad = 0;
	for( int i=0; i<gray_image.cols; i++ ){
		for( int j=0; j<gray_image.rows; j++ ){
			uchar pixel = gray_image.at< uchar >(Point(i,j));
			if( (int)pixel <= 128 ) gray_image.at< uchar >(Point(i,j)) = BLACK;
			else gray_image.at< uchar >(Point(i,j)) = WHITE;
		}
	}
}

/**
 * @tourImage
 */
void tourImage( bool filterReg ){
	TAG_COUNT = 254;
	canasta muchasSemillas;
	int largestRegion;
	for( int i=0; i<gray_image.cols; i++ ){
		for( int j=0; j<gray_image.rows; j++ ){
			uchar pixel = gray_image.at< uchar >(Point(i,j));
			if( (int)pixel == WHITE ){
				gray_image.at< uchar >(Point(i,j)) = TAG_COUNT;
				semilla coordenada = std::make_pair(i,j);
	  			muchasSemillas.push( coordenada );
	  			representante.push_back( coordenada );
					RegionsPixels.push_back( regiones8vecinos( muchasSemillas ) );	
			}
		}
	}		
	std::vector<int> v(RegionsPixels);
	std::make_heap (v.begin(),v.end());
	largestRegion = v.front();
	DIF = (TAG_COUNT - 100)/RegionsPixels.size();

	///Funcion de filtrado de Regiones
	if( filterReg )
	RegionsFilter( largestRegion );
}

/**
 * @RegionsFilter
 */
void RegionsFilter( int largestRegion ){
	canasta muchasSemillas;
	for(int i=0; i<RegionsPixels.size(); i++){
		if( (largestRegion/100)*PORCENT > RegionsPixels[i] ){
			muchasSemillas.push( representante[i] );
			regiones8vecinosBLACK( muchasSemillas );
		}
	}
	//Ya esta informacion no es util,
	//se limpia los vectores de semilla
	//y cantidad de pixeles por semilla
	RegionsPixels.clear();
	representante.clear();
}

/**
 * @showResults
 */
void showResults( ){
	if( RegionsPixels.size() != 0 )
	{
		std::cout<< "\n	Number of Regions: " << RegionsPixels.size() <<std::endl;
		for(int i=0;i<RegionsPixels.size();i++)
			std::cout<< " Region "<<i+1<<": " << RegionsPixels[i] <<" pixels" <<std::endl;
	}
}
							/*										*
							*IN THAT PART STARTS THE 8 NEIGTHBOURS	*
							*///
/**
 * @regiones8vecinos
 * int: Retorna la cantidad de pixeles según su zona segmentada
 */
int regiones8vecinos( canasta muchasSemillas ){
	int x, y, nPixels=1;//inicializa en 1 porque cuento el pixel semilla
	while( !muchasSemillas.empty( ) ){
		nPixels++;
		x = muchasSemillas.front( ).first;
		y = muchasSemillas.front( ).second;
			///neigthbour up
			if( y-1 >= 0 ){
				uchar upT = gray_image.at< uchar >(Point(x, y-1));
				if( (int)upT == WHITE ){
					//paint the neigthbour
					gray_image.at< uchar >(Point(x, y-1)) = TAG_COUNT;
					semilla coordenada = std::make_pair( x, y-1 );
		  			muchasSemillas.push( coordenada );
				}
			}
			///neigthbour down
			if( y+1 < gray_image.rows ){
				uchar downT = gray_image.at< uchar >(Point(x, y+1));
				if( (int)downT == WHITE ){		
					//paint the neigthbour
					gray_image.at< uchar >(Point(x, y+1)) = TAG_COUNT;				
					semilla coordenada = std::make_pair( x, y+1 );
			  		muchasSemillas.push( coordenada );

				}
			}
			///neigthbour left
			if( x-1 >= 0 ){
				uchar leftT = gray_image.at< uchar >(Point(x-1, y));
				if( (int)leftT == WHITE ){
					//paint the neigthbour
					gray_image.at< uchar >(Point( x-1, y )) = TAG_COUNT;			
					semilla coordenada = std::make_pair( x-1, y );
			  		muchasSemillas.push( coordenada );	
				}
			}
			///neigthbour rigth
			if( x+1 < gray_image.cols ){
				uchar rigthT = gray_image.at< uchar >(Point(x+1, y));
				if( (int)rigthT == WHITE ){		
					//paint the neigthbour
					gray_image.at< uchar >(Point( x+1, y )) = TAG_COUNT;				
					semilla coordenada = std::make_pair( x+1, y );
			  		muchasSemillas.push( coordenada );
				}
			}
			///neigthbour up-left
			if(  y-1 >= 0 && x-1 >= 0 ){
				uchar upLeftT = gray_image.at< uchar >(Point(x-1, y-1));
				if( (int)upLeftT == WHITE ){		
					//paint the neigthbour
					gray_image.at< uchar >(Point(x-1,y-1)) = TAG_COUNT;				
					semilla coordenada = std::make_pair(x-1, y-1);
			  		muchasSemillas.push( coordenada );
				}
			}
			///neigthbour down-left
			if( y+1 < gray_image.rows && x-1 >= 0 ){
				uchar downLeftT = gray_image.at< uchar >(Point(x-1, y+1));
				if( (int)downLeftT == WHITE ){		
					//paint the neigthbour
					gray_image.at< uchar >(Point(x-1, y+1)) = TAG_COUNT;				
					semilla coordenada = std::make_pair(x-1, y+1);
			  		muchasSemillas.push( coordenada );
				}
			}
			///neigthbour up-rigth
			if(  y-1 >= 0 && x+1 < gray_image.cols ){
				uchar upRigthT = gray_image.at< uchar >(Point(x+1, y-1));
				if( (int)upRigthT == WHITE ){		
					//paint the neigthbour
					gray_image.at< uchar >(Point(x+1, y-1)) = TAG_COUNT;				
					semilla coordenada = std::make_pair(x+1, y-1);
			  		muchasSemillas.push( coordenada );
				}
			}
			///neigthbour down-rigth
			if( y+1 < gray_image.rows && x+1 < gray_image.cols ){
				uchar downRigthT = gray_image.at< uchar >(Point(x+1, y+1));
				if( (int)downRigthT == WHITE ){		
					//paint the neigthbour
					gray_image.at< uchar >(Point(x+1, y+1)) = TAG_COUNT;				
					semilla coordenada = std::make_pair(x+1, y+1);
			  		muchasSemillas.push( coordenada );
				}
			}
		muchasSemillas.pop( );
	}//elihw
	TAG_COUNT = TAG_COUNT - DIF;
	return nPixels;
}

/**
 * @regiones8vecinosBLACK
 */
void regiones8vecinosBLACK(  canasta muchasSemillas ){
	int x, y;
	while( !muchasSemillas.empty( ) ){
		x = muchasSemillas.front( ).first;
		y = muchasSemillas.front( ).second;
			///neigthbour up
			if( y-1 >= 0 ){
				uchar upT = regionFilter_image.at< uchar >(Point(x, y-1));
				if( (int)upT != BLACK ){
					//paint the neigthbour
					regionFilter_image.at< uchar >(Point(x, y-1)) = BLACK;
					semilla coordenada = std::make_pair( x, y-1 );
		  			muchasSemillas.push( coordenada );
				}
			}
			///neigthbour down
			if( y+1 < regionFilter_image.rows ){
				uchar downT = regionFilter_image.at< uchar >(Point(x, y+1));
				if( (int)downT != BLACK ){		
					//paint the neigthbour
					regionFilter_image.at< uchar >(Point(x, y+1)) = BLACK;				
					semilla coordenada = std::make_pair( x, y+1 );
			  		muchasSemillas.push( coordenada );

				}
			}
			///neigthbour left
			if( x-1 >= 0 ){
				uchar leftT = regionFilter_image.at< uchar >(Point(x-1, y));
				if( (int)leftT != BLACK ){
					//paint the neigthbour
					regionFilter_image.at< uchar >(Point( x-1, y )) = BLACK;			
					semilla coordenada = std::make_pair( x-1, y );
			  		muchasSemillas.push( coordenada );	
				}
			}
			///neigthbour rigth
			if(  x+1 < regionFilter_image.cols ){
				uchar rigthT = regionFilter_image.at< uchar >(Point(x+1, y));
				if( (int)rigthT != BLACK ){		
					//paint the neigthbour
					regionFilter_image.at< uchar >(Point( x+1, y )) = BLACK;				
					semilla coordenada = std::make_pair( x+1, y );
			  		muchasSemillas.push( coordenada );
				}
			}
			///neigthbour up-left
			if(  y-1 >= 0 && x-1 >= 0 ){
				uchar upLeftT = gray_image.at< uchar >(Point(x-1, y-1));
				if( (int)upLeftT == WHITE ){		
					//paint the neigthbour
					gray_image.at< uchar >(Point(x-1,y-1)) = BLACK;				
					semilla coordenada = std::make_pair(x-1, y-1);
			  		muchasSemillas.push( coordenada );
				}
			}
			///neigthbour down-left
			if( y+1 < gray_image.rows && x-1 >= 0 ){
				uchar downLeftT = gray_image.at< uchar >(Point(x-1, y+1));
				if( (int)downLeftT == WHITE ){		
					//paint the neigthbour
					gray_image.at< uchar >(Point(x-1, y+1)) = BLACK;				
					semilla coordenada = std::make_pair(x-1, y+1);
			  		muchasSemillas.push( coordenada );
				}
			}
			///neigthbour up-rigth
			if(  y-1 >= 0 && x+1 < gray_image.cols ){
				uchar upRigthT = gray_image.at< uchar >(Point(x+1, y-1));
				if( (int)upRigthT == WHITE ){		
					//paint the neigthbour
					gray_image.at< uchar >(Point(x+1, y-1)) = BLACK;				
					semilla coordenada = std::make_pair(x+1, y-1);
			  		muchasSemillas.push( coordenada );
				}
			}
			///neigthbour down-rigth
			if( y+1 < gray_image.rows && x+1 < gray_image.cols ){
				uchar downRigthT = gray_image.at< uchar >(Point(x+1, y+1));
				if( (int)downRigthT == WHITE ){		
					//paint the neigthbour
					gray_image.at< uchar >(Point(x+1, y+1)) = BLACK;				
					semilla coordenada = std::make_pair(x+1, y+1);
			  		muchasSemillas.push( coordenada );
				}
			}
		muchasSemillas.pop( );
	}//elihw
}


							/*										*
							*IN THAT PART FINDS THE POINTS	*
							*///
/**
 * @filtroPromedio
 */
void filtroPromedio( Mat& img ){
	Mat image;
	medianBlur( img, image, 3);
	img = image;
}
/**
 * @regiones8vecinosBLACK
 */
std::vector<Point2f> findCenters( ){
	RNG rng(12345);
	Mat canny_output;
	std::vector<std::vector<Point> > contours;
  	std::vector<Vec4i> hierarchy;
	int thresh = 0;
	Canny( gray_image, canny_output, thresh, thresh*2, 3 );
	writeResult( canny_output, nombreImagen , "_5_canny.jpg" );
	findContours( canny_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_KCOS, Point(0, 0) );
	//std::cout<<contours.size()<<std::endl;
	  /// Get the moments
  std::vector<Moments> mu(contours.size() );
  for( size_t i = 0; i < contours.size(); i++ )
     { mu[i] = moments( contours[i], false ); }

  ///  Get the mass centers:
  std::vector<Point2f> mc( contours.size() );
  for( size_t i = 0; i < contours.size(); i++ )
     { mc[i] = Point2f( static_cast<float>(mu[i].m10/mu[i].m00) , static_cast<float>(mu[i].m01/mu[i].m00) ); }

 drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
  for( size_t i = 0; i< contours.size(); i++ )
     {
       Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
       //drawContours( drawing, contours, (int)i, color, 2, 8, hierarchy, 0, Point() );
       circle( drawing, mc[i], 8, color, -1, 8, 0 );
       //std::cout<<"Puntos: "<<mc[i]<<std::endl;
     }
 writeResult( drawing, nombreImagen , "_6_centroid.jpg" );
 return(mc);
}

/**
 * @crearGrafo
 */
void crearGrafo( semilla posRobot ){

	///Agregar pos del minicadie
	puntos ini;
		ini.x = posRobot.first;
		ini.y = posRobot.second;
		ini.z = 0;
	grafo.AddVertex(ini);
	///findCenters:Busca todos los Centroides de cada pelota
	std::vector<Point2f> v = findCenters();
	for(int i = 0; i < v.size(); i++){
		Point pt = (v[i])*1.f;
		//std::cout << pt.x << ", " << pt.y << std::endl;
		puntos pun;
           pun.x = pt.x;
           pun.y = pt.y;
           pun.z = 0;
           if( pun.x >= 0.f && pun.y >= 0.f )
           	grafo.AddVertex(pun);
	}
	std::vector<puntos> Pun = grafo.m_Vertices;
	//Conectar Vertices
	//Generar Grafo Completo
	float dis=0;
	for(int i = 0; i < Pun.size(); i++){
		for(int j = 0; j < Pun.size(); j++){
			if(i!=j){
			 dis=grafo.CalcularDis(i,j);
             grafo.SetArc(i,j, dis);
             grafo.SetArc(j,i, dis);
         	}
        }
	}
	camino = grafo.PrintPreorderGraph(0);
	recorrerDibujarGrafo( );
}

/**
 * @recorrerDibujarGrafo
 */
void recorrerDibujarGrafo(  ){
	std::cout<< "[Coordenadas]\n";
	Point pt1, pt2;
	for( int i = 0; i < camino.size( )-1; i++ ){
		pt1.x = camino[i].x;
		pt1.y = camino[i].y;
		std::cout<< "["<<pt1.x<<","<<pt1.y<<"]\n";
		pt2.x = camino[i+1].x;
		pt2.y = camino[i+1].y;

		MyLine( drawing, pt1, pt2 );
	}

	pt1.x = camino[ camino.size( )-1 ].x;
	pt1.y = camino[ camino.size( )-1 ].y;
	std::cout<< "["<<pt1.x<<","<<pt1.y<<"]\n";
	pt2.x = camino[0].x;
	pt2.y = camino[0].y;
	MyLine( drawing, pt1, pt2 );

	rectangle( drawing,
          Point( camino[0].x-20, camino[0].y-20),
          Point( camino[0].x+20, camino[0].y+20),
          Scalar( 0, 255, 255 ),
          -1,
          8 );
	writeResult( drawing, nombreImagen , "_7_path.jpg" );
}

/**
 * @MyLine
 */
void MyLine( Mat img, Point start, Point end )
{
	RNG rng(01234);
	Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
  int thickness = 2;
  int lineType = 8;
  line( img,
        start,
        end,
        color,
        thickness,
        lineType );
}

/**
 * @identificarPuntos
 */
void identificarPuntos( Mat img ){
	//TODO
	///Por el momento:
	///Supongo que ya se la ubicacion en posicion en pixel de la cancha
    //================//
    //  1----------2  //
    //  |          |  //
    //  |          |  //
    //  |          |  //
    //  4----------3  //
    //================//
//Cancha1.3
	fourPoints.x1 = 1525; fourPoints.y1 = 880; //1
	fourPoints.x2 = 2275; fourPoints.y2 = 1056; //2
	fourPoints.x3 = 607+5; fourPoints.y3 = 1718-5; //3
	fourPoints.x4 = 250+5; fourPoints.y4 = 1231-5; //4
}

Mat generarPerspectiva( Mat& img, double& width, double& heigth ){
	//TODO
	//Identificar los 4 Puntos de de la cancha
	identificarPuntos( img );

	//Tomando encuenta el tamaño de la cancha
	Mat imagePerspective = Mat::zeros( Size( width, heigth ), CV_8UC3 );
	// Input Quadilateral or Image plane coordinates
    Point2f inputQuad[4]; 
    // Output Quadilateral or World plane coordinates
    Point2f outputQuad[4];         
    // Lambda Matrix
    Mat lambda( 2, 4, CV_32FC1 );

    // Set the lambda matrix the same type and size as input
    lambda = Mat::zeros( img.rows, img.cols, img.type() );

    std::vector<Point> not_a_rect_shape;
    not_a_rect_shape.push_back(Point(fourPoints.x1, fourPoints.y1));	//1
    not_a_rect_shape.push_back(Point(fourPoints.x2, fourPoints.y2));	//2
    not_a_rect_shape.push_back(Point(fourPoints.x3, fourPoints.y3));	//3
    not_a_rect_shape.push_back(Point(fourPoints.x4, fourPoints.y4));	//4

    inputQuad[0] = not_a_rect_shape[0];
    inputQuad[1] = not_a_rect_shape[1];
    inputQuad[2] = not_a_rect_shape[2];
    inputQuad[3] = not_a_rect_shape[3];

    // The 4 points where the mapping is to be done , from top-left in clockwise order
    outputQuad[0] = Point2f( 0,0 );
    outputQuad[1] = Point2f( imagePerspective.cols-1,0);
    outputQuad[2] = Point2f( imagePerspective.cols-1,imagePerspective.rows-1);
    outputQuad[3] = Point2f( 0,imagePerspective.rows-1 );

    // Get the Perspective Transform Matrix i.e. lambda 
    lambda = getPerspectiveTransform( inputQuad, outputQuad );
    // Apply the Perspective Transform just found to the src image
    warpPerspective(img,imagePerspective,lambda,imagePerspective.size() );

    return imagePerspective;
}

/**
 * @segmentacion
 */
void segmentacion( Mat& img, int bins ){
  Mat hsv; Mat hue;

  /// Transform it to HSV
  cvtColor( img, hsv, COLOR_BGR2HSV );

  /// Use only the Hue value
  hue.create( hsv.size(), hsv.depth() );
  int ch[] = { 0, 0 };
  mixChannels( &hsv, 1, &hue, 1, ch, 1 );

  MatND hist;
  int histSize = MAX( bins, 2 );
  float hue_range[] = { 0, 180 };
  const float* ranges = { hue_range };

  /// Get the Histogram and normalize it
  calcHist( &hue, 1, 0, Mat(), hist, 1, &histSize, &ranges, true, false );
  normalize( hist, hist, 0, 255, NORM_MINMAX, -1, Mat() );

  /// Get Backprojection
  MatND backproj;
  calcBackProject( &hue, 1, 0, hist, backproj, &ranges, 1, true );

  /// Draw the backproj
  //imwrite( "BackProj.jpg", backproj );
  img = backproj;
/*
 * /// Draw the histogram
 
 * int w = 400; int h = 400;
 * int bin_w = cvRound( (double) w / histSize );
 * Mat histImg = Mat::zeros( w, h, CV_8UC3 );

 * for( int i = 0; i < bins; i ++ )
 *    { rectangle( histImg, Point( i*bin_w, h ), Point( (i+1)*bin_w, h - cvRound( hist.at<float>(i)*h/255.0 ) ), Scalar( 0, 0, 255 ), -1 ); }

 * imwrite( "Histogram.jpg", histImg );
*/
}