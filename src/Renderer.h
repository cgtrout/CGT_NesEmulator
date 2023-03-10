// Renderer.h: interface for the Renderer class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include < windows.h >
#include < gl\gl.h >			
#include < gl\glu.h >			
//#include < gl\glaux.h >		

#define GL_TEXTURE0_ARB                     0x84C0
#define GL_TEXTURE1_ARB                     0x84C1

// This is the number that is associated with a face that is of type "polygon"
#define FACE_POLYGON	1

//#include "vector.h"
//#include < vector >
#include < math.h >
#include "image.h"

#if _MSC_VER > 1000
#pragma once
#pragma warning( disable : 4786 ) 
#endif // _MSC_VER > 1000


/*
=================================================================
=================================================================
Renderer   Class
=================================================================
=================================================================
*/
class Renderer {
public:
	Renderer( );
	virtual ~Renderer( );

	void initialize( );

	void initFrame( );
	void renderFrame( );
	void renderLevel( );
	void render2D( );
	
	void findTextureExtension( char *strFileName );

private:
	char *getGLErrorString( int error );

	//gl error code
	int glError;
};

