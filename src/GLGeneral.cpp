//#include "precompiled.h"
#include "GLGeneral.h"

int getNextPowerOf2( int value );

/*
==============================================
createTexture()

  create an opengl texture and store its id in
  texid
==============================================
*/
void createTexture( Image *image, unsigned int *texid, GLenum format ) {
	glEnable( GL_TEXTURE_2D );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	glGenTextures( 1, texid );
	glBindTexture( GL_TEXTURE_2D, *texid );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,  GL_NEAREST );
	
	//ensure that x, and y dimensions are powers of two
	//todo - test this to ensure it has proper effect
	image->sizeX = getNextPowerOf2( image->sizeX );
	image->sizeY = getNextPowerOf2( image->sizeY );
	
	glTexImage2D( GL_TEXTURE_2D, 0, image->channels, image->sizeX, image->sizeY, 0, format, GL_UNSIGNED_BYTE, image->data.data() );
}

int pow2Table[ ] = {2,4,6,8,16,32,64,128,256,512,1024,2048};

int getNextPowerOf2( int value ) {
    static int sizeOfTable = sizeof( pow2Table )/sizeof( int );
    
    for( int x = 0; x < sizeOfTable; x++ ) {
		if( value < pow2Table[ x ] ) {
			return pow2Table[ x ];
		}
		if( value == pow2Table[ x ] ) {
			return pow2Table[ x ];
		}
	}
	
	return 0;
}
