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
void createTexture( Image image, unsigned int *texid, GLenum format ) {
	glEnable( GL_TEXTURE_2D );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	glGenTextures( 1, texid );
	glBindTexture( GL_TEXTURE_2D, *texid );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,  GL_NEAREST );
	
	//ensure that x, and y dimensions are powers of two
	image.resizePowerOfTwo( );
	
	glTexImage2D( GL_TEXTURE_2D, 0, image.channels, image.sizeX, image.sizeY, 0, format, GL_UNSIGNED_BYTE, image.data.data() );
}


