#if !defined( GLGENERAL_INC )
#define GLGENERAL_INC

#include < windows.h >

#include < gl\gl.h >			// Header File For The OpenGL32 Library
#include < gl\glu.h >			// Header File For The GLu32 Library

#include "Image.h"

//creates opengl texture
void createTexture( Image image, unsigned int *texid, GLenum format = GL_RGBA );

#endif
