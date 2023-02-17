// Renderer.cpp: implementation of the Renderer class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning( disable : 4786 )

#include "Renderer.h"
#include "GameMain.h"
extern GameMain *gm;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Renderer::Renderer( ) {

}

Renderer::~Renderer( ) {
}

void Renderer::initialize( ) {
	

}


/*
==============================================
Renderer::initFrame( )

  initializes a frame
==============================================
*/
void Renderer::initFrame( ) {
	// Clear The Screen And The Depth Buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	//ensure correct matrix mode is selected for next frame
	glMatrixMode( GL_MODELVIEW );

	// Reset The matrix
	glLoadIdentity( );	
}

/*
==============================================
Renderer::renderFrame( )
==============================================
*/
void Renderer::renderFrame( ) {
	render2D( );
	
#ifdef _DEBUG
	//see if there were any errors this frame
	int glError = glGetError( );
	static int numErrors;
	numErrors = 0;
	if( glError != GL_NO_ERROR ) {
		while( glError != GL_NO_ERROR ) {
			_log->Write( "gl error: %s @ %f", getGLErrorString( glError ), gm->getCurrTime( ) );
			glError = glGetError( );
			numErrors++;
		}		
		if( numErrors > 1 )
			_log->Write( "" );
	}
#endif
}

/*
==============================================
Renderer::render2D( )

  Render 2d aspects of the scene
==============================================
*/
void Renderer::render2D( ) {
	//disable multitexuring so they don't affect 2d drawing
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisable( GL_TEXTURE_2D );
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisable( GL_TEXTURE_2D );

}


/*
==============================================
findTextureExtension( )

  finds and appends correct file extension
  for texture files
==============================================
*/
void Renderer::findTextureExtension( char *strFileName ) {
	char strJPGPath[ MAX_PATH ] = {0};
	char strTGAPath[ MAX_PATH ]    = {0}; 
	FILE *fp = NULL;

	// Get the current path we are in
	GetCurrentDirectory( MAX_PATH, strJPGPath );

	// Add on a '\' and the file name to the end of the current path.
	// We create 2 seperate strings to test each image extension.
	strcat( strJPGPath, "\\" );
	strcat( strJPGPath, strFileName );
	strcpy( strTGAPath, strJPGPath );
	
	// Add the extensions on to the file name and path
	strcat( strJPGPath, ".jpg" );
	strcat( strTGAPath, ".tga" );

	// Check if there is a jpeg file with the texture name
	if( ( fp = fopen( strJPGPath, "rb" ) ) != NULL ) {
		// If so, then let's add ".jpg" onto the file name and return
		strcat( strFileName, ".jpg" );
		return;
	}

	// Check if there is a targa file with the texture name
	if( ( fp = fopen( strTGAPath, "rb" ) ) != NULL ) {
		// If so, then let's add a ".tga" onto the file name and return
		strcat( strFileName, ".tga" );
		return;
	}
}

char *Renderer::getGLErrorString( int error ) {
	switch( error ) {
		case GL_NO_ERROR:			return "GL_NO_ERROR";
		case GL_INVALID_ENUM:		return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:		return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:	return "GL_INVALID_OPERATION";
		case GL_STACK_OVERFLOW:		return "GL_STACK_OVERFLOW";
		case GL_STACK_UNDERFLOW:	return "GL_STACK_UNDERFLOW";
		case GL_OUT_OF_MEMORY:		return "GL_OUT_OF_MEMORY";

		default:					return "Unknown GL Error";
	}
}

