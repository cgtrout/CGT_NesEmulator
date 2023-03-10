//original author - Ben Humphrey ( DigiBen )

#pragma once
#include <vector>

typedef unsigned char ubyte;

#include "CgtException.h"
//2 dimensional vector 
struct Vec2d {
	int x, y;

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
	Pixel3Byte() { } 
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
	
	void plotPixel( const Vec2d *pos, const Pixel3Byte *color, ubyte alpha = 255 );
	void clearImage();
	
	//allocate image based on settings
	void allocate();
	
	bool isAllocated() { return !data.empty(); }

	ubyte *getData() { return data.data(); }
	void setData( ubyte* data );

	int getSize() { return channels * sizeX * sizeY; }
	
	//size up to next power of two to satisfy opengl requirements
	void resizePowerOfTwo( );

	Image(): data(), imgid(0) {}
	~Image();

	//TODO eventually this should probably be private
	std::vector<ubyte> data;	
	unsigned int imgid;
};

class ImageException : public CgtException {
public:	
	ImageException( const char *h, const char *m, bool show = true) {
		::CgtException(h, m, show);
	}
}; 

void copyImage( Image *source, int sx, int sy, int width, int height, Image *destination, int dx, int dy );
Image loadImage( std::string_view strFileName );
Image *flipImage( Image *image );
Image convertToAlpha( int aR, int aG, int aB, Image image );

// This loads and returns the BMP data
Image LoadBMP( std::string_view strFileName );

void ExportImageToBMP( const Image &image, std::string_view fileName );

// This loads and returns the TGA data
//Image *LoadTGA( const char *strFileName );

// This loads and returns the JGP data
Image *LoadJPG( const char *strFileName );

// This decompresses the JPEG and fills in the image data
//void DecodeJPG( jpeg_decompress_struct* cinfo, Image *pImageData );

