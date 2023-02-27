#if !defined( CGTGraph_INCLUDED )
#define CGTGraph_INCLUDED

#include "ImageTools.h"

/*
================================================================
================================================================
Class GraphGen

  generates a custom graph and outputs it
  as a rgb bitmap 

  black will be considered the alpha color
================================================================
================================================================
*/

enum GraphType {
	GraphType_point,
	GraphType_line
};

template< class UnitType >
class GraphGen {
public:
	GraphGen() : gridSize_x( 1 ), gridSize_y( 1 ) {
		image.sizeX = 50;
		image.sizeY = 50;
		image.channels = 4;
		//image.data = 0;
		lineDrawer.setImage( &image );
		pointDrawer.setImage( &image );
		pointDrawer.radius = 1;
		pointDrawer.color = Pixel3Byte( 255, 255, 0 );
		graphType = GraphType_point;
	}


	//set axis positioning 
	void setWindow_x( int start, int end );
	void setWindow_y( UnitType start, UnitType end );

	void setDrawGrid_x( bool b );
	void setDrawGrid_y( bool b );

	void setGridSize_x( int );
	void setGridSize_y( UnitType );

	void setPixelWidth( int val );
	void setPixelHeight( int val );

	int getPixelWidth();
	int getPixelHeight();

	//gives this class access to a data buffer (which is 
	//what is graphed)
	void setDataBuffer( UnitType *data, int size );

	void setAxisColor( Pixel3Byte *color );
	void setGridColor( Pixel3Byte *color );
	void setLineColor( Pixel3Byte *color );
	void setShadeColor( Pixel3Byte *color );

	//should the graph be shaded under the "line"
	void setShadeGraph( bool );

	//get byte size of generated bitmap
	int getGraphImageSize();
	
	//generate and return pointer to generated bitmap
	Image *getImage();

private:
	//pointer to buffer containing data
	UnitType *dataBuffer;
	
	//size of dataBuffer
	int dataSize;

	//start and end of x dimension
	int startPos_x, endPos_x;
	
	//start and end of y dimension
	UnitType startPos_y, endPos_y;
	
	//draw a grid for x and y dimensions?
	bool drawGrid_x, drawGrid_y;
	
	//distance between grid marks( in data coordinates )
	int gridSize_x;
	UnitType gridSize_y;

	Pixel3Byte axisColor;
	Pixel3Byte gridColor;
	Pixel3Byte lineColor;
	Pixel3Byte shadeColor;

	//genated bitmap
	Image image;

	PointDraw pointDrawer;
	LineDraw lineDrawer;

	void createGraph();
	void allocateImage();
	
	//calculates how many samples each pixel represents
	void calcDataWidthPerPixel();
	float dataWidthPerPixel;

	void calcDataHeightPerPixel();
	float dataHeightPerPixel;

	//finds the nearest pixel based on data position given
	int findNearestPixel_x( int dataPos );
	int findNearestPixel_y( UnitType dataPos );

	//draws various elements to graph
	void placeAxis_x();
	void placeAxis_y();
	void placeGrid_x();
	void placeGrid_y();
	void placePlotData();

	//has the bitmap been generated?
	bool imageGenerated;
	
	//shade under plotted area?
	bool shadeGraph;		

	GraphType graphType;
};

//
//
//basic getter setter functions
//
//
template< class UnitType >
inline Image *GraphGen< UnitType >::getImage() {
	createGraph();
	return &image;
}

template< class UnitType >
inline void GraphGen< UnitType >::setWindow_x( int start, int end ) {
	startPos_x = start;
	endPos_x = end;
}

template< class UnitType >
inline void GraphGen< UnitType >::setWindow_y( UnitType start , UnitType end ) {
	startPos_y = start;
	endPos_y = end;
}

template< class UnitType >
inline void GraphGen< UnitType >::setDrawGrid_x( bool b ) {
	drawGrid_x = b;
}

template< class UnitType >
inline void GraphGen< UnitType >::setDrawGrid_y( bool b ) {
	drawGrid_y = b;
}

template< class UnitType >
inline void GraphGen< UnitType >::setGridSize_x( int val ) {
	gridSize_x = val;
}

template< class UnitType >
inline void GraphGen< UnitType >::setGridSize_y( UnitType val ) {
	gridSize_y = val;
}

template< class UnitType >
inline void GraphGen< UnitType >::setPixelWidth( int val ) {
	image.sizeX = val;	
}

template< class UnitType >
inline void GraphGen< UnitType >::setPixelHeight( int val ) {
	image.sizeY = val;
}

template< class UnitType >
inline int GraphGen< UnitType >::getPixelWidth() {
	return image.sizeX;
}

template< class UnitType >
inline int GraphGen< UnitType >::getPixelHeight() {
	return image.sizeY;
}

template< class UnitType >
inline void GraphGen< UnitType >::setDataBuffer( UnitType *data, int size ) {
	dataBuffer	= data;
	dataSize	= size;
}

template< class UnitType >
inline void GraphGen< UnitType >::setAxisColor( Pixel3Byte *color ) {
	axisColor = *color;
}


template< class UnitType >
inline void GraphGen< UnitType >::setGridColor( Pixel3Byte *color ) {
	gridColor = *color;
}


template< class UnitType >
inline void GraphGen< UnitType >::setLineColor( Pixel3Byte *color ) {
	lineColor = *color;
}

template< class UnitType >
inline void GraphGen< UnitType >::setShadeColor( Pixel3Byte *color ) {
	shadeColor = *color;
}

template< class UnitType >
inline void GraphGen< UnitType >::setShadeGraph( bool val) {
	shadeGraph = val;
}

template< class UnitType >
inline int GraphGen< UnitType >::getGraphImageSize() {
	if( !imageGenerated ) {
		return 0;
	}
	return image.getSize();
}


template< class UnitType >
inline void GraphGen< UnitType >::allocateImage() {
	image.allocate();
	image.clearImage();
}

#include "Graph.imp"

class GraphException : public CgtException {
public:	
	GraphException( char *h, char *m, bool show = true) {
		::CgtException(h, m, show);
	}
}; 

#endif 
