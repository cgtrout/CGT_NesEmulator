#include "image.h"
#include < windows.h >
#include < fstream >
#include <assert.h>

//TODO change all loading functions so that they take in an image rather than
//returning one

/* 
==============================================
Image::~Image()
==============================================
*/
Image::~Image() {
	//if( data != 0 ) {
		//delete[] data;
	//}
}

/* 
==============================================
void Image::plotPixel( const Vec2d *pos, const Pixel3Byte *color, const ubyte alpha )
==============================================
*/
void Image::plotPixel( const Vec2d *pos, const Pixel3Byte *color, const ubyte alpha ) {
	int bpos = ( pos->y * sizeX * channels ) + ( pos->x * channels );
	int imgSize = this->getSize();
	_ASSERT( bpos < imgSize );

	data[ bpos++ ] = color->color[ 0 ];
	data[ bpos++ ] = color->color[ 1 ];
	data[ bpos   ] = color->color[ 2 ];

	if( channels == 4 ) {
		data[ ++bpos ] = alpha;
	}
}

/* 
==============================================
void Image::clearImage()
==============================================
*/
void Image::clearImage() {
	memset( data.data(), 0, getSize() );
}

/* 
==============================================
void Image::allocate()
==============================================
*/
void Image::allocate() {
	data.resize( getSize( ) );
}

void Image::setData( ubyte* data ) {
	assert( channels != 0 && sizeX != 0 && sizeY != 0 );
	this->data = std::vector<ubyte>( data, data + getSize());
}

/* 
==============================================
Image *loadImage( const char *strFileName )

  TODO add loaders for different file types
==============================================
*/
Image *loadImage( const char *strFileName ) {
	Image *pImage;

	// If the file is a bitmap, load the bitmap and store the data in pImage
	if( strstr( strFileName, ".bmp" ) ) {
		pImage = LoadBMP( strFileName );
	}
	// Else we don't support the file format that is trying to be loaded
	else {
		throw ImageException( "loadImage()", "unsupported file format" );
	}

	return pImage;
}

/*
==============================================
flipImage( Image *image )

  flips an image around vertically
==============================================
*/
Image *flipImage( Image *image ) {
	Image *newImage = NULL;
	
	newImage = new Image;

	newImage->sizeX = image->sizeX;
	newImage->sizeY = image->sizeY;
	newImage->channels = image->channels;

	newImage->setData( image->data.data() );

	int width = image->sizeX * image->channels;

	int oldimagecount = 0;
	int newimagecount = 0;	
	int y;

	for( int x = image->sizeY-1; x >= 0; x--, oldimagecount = x * width ) {
		for( y = 0; y < width; y++ ) {
			newImage->data[ newimagecount++ ] = image->data[ oldimagecount++ ];
		}
	}
	
	delete image;

	image = newImage;
	return image;
}

/*
==============================================
convertToAlpha()

  converts an image without an alpha channel to one with a
  alpha channel created based on colors given as parameters.

  Color selected by parameters will be set as transparent in the 
  alpha channel
==============================================
*/
Image *convertToAlpha( int aR, int aG, int aB, Image *image ) {
	Image *newImage = NULL;
	int sizeOfImage = image->sizeX * image->sizeY;

	if( image->channels != 3 ) {
		throw ImageException( "convertToAlpha()", "Image must have 3 channels ( RGB )" );
	}

	newImage = new Image;
	
	newImage->channels = 4;
	newImage->sizeX = image->sizeX;
	newImage->sizeY = image->sizeY;
	newImage->setData( image->data.data() );
	
	int oldimagecount = 0;	
	int newimagecount = 0;	
	ubyte red, green, blue, alpha;
	for ( int x = 0; x < sizeOfImage; x++, oldimagecount += 3, newimagecount +=4 ) {
		red = image->data[ oldimagecount ];
		green = image->data[ oldimagecount+1 ];
		blue = image->data[ oldimagecount+2 ];
		alpha = ( red == aR && green == aG && blue == aB ) ? 0 : 255;

		newImage->data[ newimagecount ] = red;
		newImage->data[ newimagecount+1 ] = green;
		newImage->data[ newimagecount+2 ] = blue;
		newImage->data[ newimagecount+3 ] = alpha;		
	}
	
	delete image;

	image = newImage;
	return image;
}

//helper function for LoadBMP - loads an 'L.S.Byte first' format int from file
//file must be loaded
unsigned int readLSBFirstInt( std::ifstream *is ) {
    _ASSERT( is != NULL );

	ubyte tempbyte[4];
	unsigned int value;
    
	for( int x = 0; x < 4; x++ ) {
        *is >> tempbyte[ x ];
    }
    //reverse
    value = tempbyte[ 0 ] + ( tempbyte[ 1 ] << 8 ) + ( tempbyte[ 2 ] << 16 ) + ( tempbyte[ 3 ] << 24 );
    return value;    
}

//only supports 24-bit bmps
//does not take into account padding bytes if bmp dimensions x and y are not a multiple of 4
Image *LoadBMP( const char *strFileName ) {
    std::string error;
    std::ifstream is( strFileName, std::ios::binary );
    
    //Image to return
    Image *t;
    
    int dataSize;
    
    if( !is ) {
        error = "Couldn't load BMP File: ";
        goto imageError;
    }
    
    //verify file
    char bmString[ 3 ];
    int fileSize;
    int dataOffset;
    ubyte bitCount;

	is.read( reinterpret_cast< char* >( bmString ), 2 );       
    bmString[ 2 ] = '\0';
    
    if( strcmp( bmString, "BM" ) != 0 ) {
        error = "Not a 24bit BMP File: ";
        goto imageError;
    }
    t = new Image();
    
    fileSize = readLSBFirstInt( &is );
    
    is.seekg( 0xA, std::ios::beg );
    dataOffset = readLSBFirstInt( &is );
    
    is.seekg( 0x12, std::ios::beg );
    t->sizeX = readLSBFirstInt( &is );
    
    is.seekg( 0x16, std::ios::beg );
    t->sizeY = readLSBFirstInt( &is );
    
    is.seekg( 0x1C, std::ios::beg );   
    is.read( reinterpret_cast< char* >( &bitCount ), 1 );       
     

    //bitcount must be 24
    if( bitCount != 24 ) {
        error = "Not a 24bit BMP File: ";
        goto imageError;
    }
	
	t->channels = 3;

    //get data
    is.seekg( dataOffset, std::ios::beg );
    
    dataSize = fileSize - dataOffset;
	t->allocate( );
    
    //read in data
    is.read( reinterpret_cast< char* >( t->data.data() ), dataSize );       
    
    
	//swap around 1st and 3rd byte for every 3 bytes in data
	for( int x = 0; x < dataSize ; x += 3 ) {
		ubyte temp1 = t->data[x];
		ubyte temp2 = t->data[x + 2];

		t->data[x] = temp2;
		t->data[x + 2] = temp1;
	}

    is.close();
    
    return t;
    
imageError:
	error += strFileName;
	throw ImageException( "LoadBMP()", "error.c_str()" );
}

void saveLSB_int( std::ofstream *os, unsigned int val ) {
	_ASSERT( os != NULL );

	*os << (ubyte)( ( val & 0x000000FF ) >> 0 )
		<< (ubyte)( ( val & 0x0000FF00 ) >> 8 )
		<< (ubyte)( ( val & 0x00FF0000 ) >> 16 )
		<< (ubyte)( ( val & 0xFF000000 ) >> 24 );
}

void saveLSB_uword( std::ofstream *os, unsigned int val ) {
	_ASSERT( os != NULL );

	*os << (ubyte)( ( val & 0x00FF ) >> 0 )
		<< (ubyte)( ( val & 0xFF00 ) >> 8 );
}

//NOTE: alpha information is not saved to file
void ExportImageToBMP( Image *image, char *fileName ) {
	std::ofstream *os = new std::ofstream( fileName, std::ios::binary );

	unsigned int fileSize = 0x35 + ( image->sizeX * image->sizeY * 3 );

	*os << "BM";
	
	//size
	saveLSB_int( os, fileSize );

	//bfReserved1 and 2
	saveLSB_int( os, 0 );
	
	//offset to data
	saveLSB_int( os, 0x36 );

	//size of bitmapinfoheader
	saveLSB_int( os, 40 );

	//width
	saveLSB_int( os, image->sizeX );
	
	//height
	saveLSB_int( os, image->sizeY );

	//biplanes
	saveLSB_uword( os, 0 );

	//bitcount
	saveLSB_uword( os, 24 );

	//compression
	saveLSB_int( os, 0 );

	//size of image data
	//set to zero since no there is no compression
	saveLSB_int( os, 0 );

	//unused attributes
	saveLSB_int( os, 0 );
	saveLSB_int( os, 0 );
	saveLSB_int( os, 0 );
	saveLSB_int( os, 0 );

	//now save data
	int numPixels = image->sizeX * image->sizeY;
	for( int p = 0; p < numPixels; ++p ) {
		int offset = p * image->channels;
		*os << image->data[ offset + 2 ]
			<< image->data[ offset + 1 ]
			<< image->data[ offset + 0 ];
	}

	os->close();
    delete os;
    
}
