#include "image.h"
#include <fstream>
#include <assert.h>
#include <Windows.h>
#include <gl/GL.h>

/*
==============================================
Image Move Constructor
==============================================
*/
Image::Image( Image&& other)
{
	this->data = std::move( other.data );
	this->sizeX = other.sizeX;
	this->sizeY = other.sizeY;
	this->channels = other.channels;
	this->handle = other.handle;
	other.handle = 0;
}

/*
==============================================
Image Move Assignment
==============================================
*/
Image& Image::operator=( Image&& other ) {
	this->data = std::move( other.data );
	this->sizeX = other.sizeX;
	this->sizeY = other.sizeY;
	this->channels = other.channels;
	this->handle = other.handle;
	other.handle = 0;

	return *this;
}

/*
==============================================
Image Copy Constructor
==============================================
*/
/*
Image& Image::operator=( const Image& other ) {
	this->data = std::move( other.data );
	this->sizeX = other.sizeX;
	this->sizeY = other.sizeY;
	this->channels = other.channels;
	this->handle = other.handle;
	//other.handle = 0;
}*/

/*
==============================================
Image::~Image()
==============================================
*/
Image::~Image() {
	if ( handle ) {
		glDeleteTextures( 1, &handle );
	}
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
	assert( channels != 0 && sizeX != 0 && sizeY != 0 );
	memset( data.data(), 0, getSize() );
}

/* 
==============================================
void Image::allocate()
==============================================
*/
void Image::allocate() {
	assert( channels != 0 && sizeX != 0 && sizeY != 0 );
	data.resize( getSize( ), 0 );
}

void Image::setData( ubyte* data ) {
	assert( channels != 0 && sizeX != 0 && sizeY != 0 );
	this->data = std::vector<ubyte>( data, data + getSize());
}

void Image::createGLTexture(  ) {
	if ( handle ) {
		glDeleteTextures( 1, &handle );
	}
	
	GLenum format = 0;
	if ( channels == 3 ) {
		format = GL_RGB;
	} else if ( channels == 4 ) {
		format = GL_RGBA;
	} else {
		throw CgtException( "Image::createGLTexture", "Invalid channel count", true );
	}

	glEnable( GL_TEXTURE_2D );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glGenTextures( 1, &handle );
	glBindTexture( GL_TEXTURE_2D, handle );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	//ensure that x, and y dimensions are powers of two
	resizePowerOfTwo( );

	glTexImage2D( GL_TEXTURE_2D, 0, channels, sizeX, sizeY, 0, format, GL_UNSIGNED_BYTE, data.data( ) );
}

void Image::bindGLTexture( ) {
	glBindTexture( GL_TEXTURE_2D, handle);
}

int pow2Table[] = { 2,4,6,8,16,32,64,128,256,512,1024,2048 };

int getNextPowerOf2( int value ) {
	static int sizeOfTable = sizeof( pow2Table ) / sizeof( int );

	for ( int x = 0; x < sizeOfTable; x++ ) {
		if ( value < pow2Table[ x ] ) {
			return pow2Table[ x ];
		}
		if ( value == pow2Table[ x ] ) {
			return pow2Table[ x ];
		}
	}

	return 0;
}

void Image::resizePowerOfTwo( ) {
	sizeX = getNextPowerOf2( sizeX );
	sizeY = getNextPowerOf2( sizeY );
	allocate( );
}

/* 
==============================================
loadImage

  TODO add loaders for different file types
==============================================
*/
Image loadImage( std::string_view strFileName ) {
	Image pImage;

	// If the file is a bitmap, load the bitmap and store the data in pImage
	if( strstr( strFileName.data(), ".bmp" ) ) {
		pImage = LoadBMP( strFileName );
	}
	// Else we don't support the file format that is trying to be loaded
	else {
		throw ImageException( "loadImage()", "unsupported file format" );
	}
	pImage = convertToAlpha( 0, 0, 0, pImage );

	pImage.createGLTexture( );

	return pImage;
}

/*
==============================================
flipImage( Image *image )

  flips an image around vertically
==============================================
*/
Image flipImage( Image image ) {
	Image newImage;
	
	newImage.sizeX = image.sizeX;
	newImage.sizeY = image.sizeY;
	newImage.channels = image.channels;

	newImage.setData( image.data.data() );

	int width = image.sizeX * image.channels;

	int oldimagecount = 0;
	int newimagecount = 0;	
	int y;

	for( int x = image.sizeY-1; x >= 0; x--, oldimagecount = x * width ) {
		for( y = 0; y < width; y++ ) {
			newImage.data[ newimagecount++ ] = image.data[ oldimagecount++ ];
		}
	}
	
	return newImage;
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
Image convertToAlpha( int aR, int aG, int aB, const Image& image ) {
	Image newimage;
	int sizeOfImage = image.sizeX * image.sizeY;

	if( image.channels != 3 ) {
		throw ImageException( "convertToAlpha()", "Image must have 3 channels ( RGB )" );
	}

	newimage.channels = 4;
	newimage.sizeX = image.sizeX;
	newimage.sizeY = image.sizeY;
	newimage.allocate( );
	
	int oldimagecount = 0;	
	int newimagecount = 0;	
	ubyte red, green, blue, alpha;
	for ( int x = 0; x < sizeOfImage; x++, oldimagecount += 3, newimagecount +=4 ) {
		red = image.data[ oldimagecount ];
		green = image.data[ oldimagecount+1 ];
		blue = image.data[ oldimagecount+2 ];
		alpha = ( red == aR && green == aG && blue == aB ) ? 0 : 255;

		newimage.data[ newimagecount ] = red;
		newimage.data[ newimagecount+1 ] = green;
		newimage.data[ newimagecount+2 ] = blue;
		newimage.data[ newimagecount+3 ] = alpha;		
	}
	
	return newimage;
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
Image LoadBMP( std::string_view strFileName ) {
    std::string error;
    std::ifstream is( strFileName.data(), std::ios::binary );
    
    //Image to return
    Image img;
    
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
    
    fileSize = readLSBFirstInt( &is );
    
    is.seekg( 0xA, std::ios::beg );
    dataOffset = readLSBFirstInt( &is );
    
    is.seekg( 0x12, std::ios::beg );
    img.sizeX = readLSBFirstInt( &is );
    
    is.seekg( 0x16, std::ios::beg );
    img.sizeY = readLSBFirstInt( &is );
    
    is.seekg( 0x1C, std::ios::beg );   
    is.read( reinterpret_cast< char* >( &bitCount ), 1 );       

    //bitcount must be 24
    if( bitCount != 24 ) {
        error = "Not a 24bit BMP File: ";
        goto imageError;
    }
	
	img.channels = 3;

    //get data
    is.seekg( dataOffset, std::ios::beg );
    
    dataSize = fileSize - dataOffset;
	img.allocate( );
    
    //read in data
    is.read( reinterpret_cast< char* >( img.data.data() ), dataSize );       
        
	//swap around 1st and 3rd byte for every 3 bytes in data
	for( int x = 0; x < dataSize ; x += 3 ) {
		ubyte temp1 = img.data[x];
		ubyte temp2 = img.data[x + 2];

		img.data[x] = temp2;
		img.data[x + 2] = temp1;
	}

    is.close();
    
    return img;
    
imageError:
	error += strFileName;
	throw ImageException( "LoadBMP()", "error.c_str()" );
}

void saveLSB_int( std::ofstream &os, unsigned int val ) {
	 os << (ubyte)( ( val & 0x000000FF ) >> 0 )
		<< (ubyte)( ( val & 0x0000FF00 ) >> 8 )
		<< (ubyte)( ( val & 0x00FF0000 ) >> 16 )
		<< (ubyte)( ( val & 0xFF000000 ) >> 24 );
}

void saveLSB_uword( std::ofstream &os, unsigned int val ) {
	 os << (ubyte)( ( val & 0x00FF ) >> 0 )
		<< (ubyte)( ( val & 0xFF00 ) >> 8 );
}

//NOTE: alpha information is not saved to file
void ExportImageToBMP( const Image &image, std::string_view fileName ) {
	std::ofstream os( fileName.data(), std::ios::binary );

	unsigned int fileSize = 0x35 + ( image.sizeX * image.sizeY * 3 );

	os << "BM";
	
	//size
	saveLSB_int( os, fileSize );

	//bfReserved1 and 2
	saveLSB_int( os, 0 );
	
	//offset to data
	saveLSB_int( os, 0x36 );

	//size of bitmapinfoheader
	saveLSB_int( os, 40 );

	//width
	saveLSB_int( os, image.sizeX );
	
	//height
	saveLSB_int( os, image.sizeY );

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
	int numPixels = image.sizeX * image.sizeY;
	for( int p = 0; p < numPixels; ++p ) {
		int offset = p * image.channels;
		os << image.data[ offset + 2 ]
			<< image.data[ offset + 1 ]
			<< image.data[ offset + 0 ];
	}

	os.close();
}
