#include < windows.h >		// Header File For Windows
#include < stdio.h >			// Header File For Standard Input/Output
#include < gl\gl.h >			// Header File For The OpenGL32 Library
#include < gl\glu.h >			// Header File For The GLu32 Library
//#include < gl\glaux.h >		// Header File For The Glaux Library

//identifier was truncated to '255' characters in the debug information
#pragma warning( disable : 4786 ) 

//minor error in .SBR file '.\Debug\GUI.sbr' ignored
#pragma warning( disable : 4503 ) 

typedef void ( APIENTRY * PFNGLACTIVETEXTUREARBPROC ) ( GLenum target );
typedef void ( APIENTRY * PFNGLCLIENTACTIVETEXTUREARBPROC ) ( GLenum texture );

// These are our defines for the multitexturing functions
PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB ;

HWND		hWnd=NULL;		// Holds Our Window Handle


//todo move all this to Renderer class
GLvoid ReSizeGLScene( GLsizei width, GLsizei height );
int InitGL( GLvoid );
int DrawGLScene( GLvoid );
GLvoid KillGLWindow( GLvoid );
BOOL CreateGLWindow( char* title, int width, int height, int bits, bool fullscreenflag );

