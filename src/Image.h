//original author - Ben Humphrey ( DigiBen )

#pragma once
#include <vector>

typedef unsigned char ubyte;

#include "CgtException.h"
//2 dimensional vector 
struct Vec2d {
	int x, y;

	//get float value for things that need it
	float getX( ) { return static_cast<float>( x ); }
	float getY( ) { return static_cast<float>( y ); }

	Vec2d( int x_, int y_ ) { x = x_; y = y_; }
};

enum { COLOR_RED, COLOR_GREEN, COLOR_BLUE };

struct Pixel3Byte {
	ubyte color[ 3 ];

	Pixel3Byte( ubyte r, ubyte g, ubyte b ) {
		color[ COLOR_RED ]		= r ;
		color[ COLOR_GREEN ]	= g;
		color[ COLOR_BLUE ]		= b;
	}
	Pixel3Byte() :
		Pixel3Byte (0, 0, 0) { } 
};

/*
================================================================
================================================================
Class Image
================================================================
================================================================
*/
class Image {
  public:
	int channels = 0;			
	int sizeX = 0;				
	int sizeY = 0;	

	float getSizeX( ) { return static_cast<float>( sizeX ); }
	float getSizeY( ) { return static_cast<float>( sizeY ); }

	unsigned int handle = 0;
	
	void plotPixel( const Vec2d *pos, const Pixel3Byte *color, ubyte alpha = 255 );
	void clearImage();
	
	//allocate image based on settings
	void allocate();
	
	bool isAllocated() { return !data.empty(); }

	ubyte *getData() { return data.data(); }
	void setData( ubyte* data );

	int getSize() { return channels * sizeX * sizeY; }

	void createGLTexture( );
	void bindGLTexture( );
	
	//size up to next power of two to satisfy opengl requirements
	void resizePowerOfTwo( );

	Image(): data() {}
	Image( Image& ) = delete;
	Image( Image&& );
	Image& operator=( Image&& );

	~Image();

	//TODO eventually this should probably be private
	std::vector<ubyte> data;	
};

class ImageException : public CgtException {
public:	
	ImageException( std::string_view h, std::string_view m, bool show = true) : 
		CgtException(h, m, show)
	{}
}; 

void copyImage( Image *source, int sx, int sy, int width, int height, Image *destination, int dx, int dy );
Image loadImage( std::string_view strFileName );
Image *flipImage( Image *image );
Image convertToAlpha( int aR, int aG, int aB, const Image& image );

// This loads and returns the BMP data
Image LoadBMP( std::string_view strFileName );

void ExportImageToBMP( const Image &image, std::string_view fileName );
