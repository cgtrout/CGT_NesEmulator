//original author - Ben Humphrey ( DigiBen )

#pragma once

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
	int channels;			
	int sizeX;				
	int sizeY;				
	
	void plotPixel( const Vec2d *pos, const Pixel3Byte *color, ubyte alpha = 255 );
	void clearImage();
	
	//allocate image based on settings
	void allocate();
	
	bool isAllocated() { return data != 0; }

	ubyte *getData() { return data; }

	int getSize() { return channels * sizeX * sizeY; }

	Image(): data(0), imgid(0) {}
	~Image();

	//TODO eventually this should probably be private
	ubyte *data;	
	unsigned int imgid;
};

class ImageException : public CgtException {
public:	
	ImageException( char *h, char *m, bool show = true) {
		::CgtException(h, m, show);
	}
}; 

void copyImage( Image *source, int sx, int sy, int width, int height, Image *destination, int dx, int dy );
Image *loadImage( const char *strFileName );
Image *flipImage( Image *image );
Image *convertToAlpha( int aR, int aG, int aB, Image *image );

// This loads and returns the BMP data
Image *LoadBMP( const char *strFileName );

void ExportImageToBMP( Image *image, char *fileName );

// This loads and returns the TGA data
//Image *LoadTGA( const char *strFileName );

// This loads and returns the JGP data
Image *LoadJPG( const char *strFileName );

// This decompresses the JPEG and fills in the image data
//void DecodeJPG( jpeg_decompress_struct* cinfo, Image *pImageData );

