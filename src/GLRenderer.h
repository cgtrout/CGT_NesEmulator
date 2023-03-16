// Renderer.h: interface for the Renderer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined( AFX_RENDERER_H__CDCC853B_3C74_4A4E_9339_2D8A8A00C624__INCLUDED_ )
#define AFX_RENDERER_H__CDCC853B_3C74_4A4E_9339_2D8A8A00C624__INCLUDED_

#include <windows.h>
#include <gl\gl.h>			
#include <gl\glu.h>			

#define GL_TEXTURE0_ARB                     0x84C0
#define GL_TEXTURE1_ARB                     0x84C1

// This is the number that is associated with a face that is of type "polygon"
#define FACE_POLYGON	1

//#include "vector.h"
//#include < vector >
#include <cmath>
#include "image.h"

#if _MSC_VER > 1000
#pragma once
//#pragma warning( disable : 4786 ) 
#endif // _MSC_VER > 1000

//x and y screen dimensions
#define NTSC_X 256
#define NTSC_Y 240

#include "CGTSingleton.h"
using namespace CGTSingleton;

#include "NesPPU.h"
#include "NesPpuTools.h"

using namespace NesEmulator;
namespace Render {
	
	/*
	=================================================================
	=================================================================
	PPUDraw Class

	  used to draw ppu pixel output to screen
	=================================================================
	=================================================================
	*/
	class PPUDraw {
	public:
		PPUDraw();

		void initialize( );
		
		//renders ppu patterntable for debugging purposes
		//F added to end to aviod name collision with console vars
		Image& drawPatternTableF();

		//draws raw pixel data to screen RGB pixel data expected
		//F added to end to aviod name collision with console vars
		void drawOutput( ubyte *data );

		Image& drawPaletteTableF( PpuSystem::NesPalette *p );
	private:
		PpuSystem::NesPPUPixelGen ppuPixelGen;
		PpuSystem::NesPPUPaletteGen paletteGen;
	};
	/*
	=================================================================
	=================================================================
	Renderer   Class
	=================================================================
	=================================================================
	*/
	class Renderer  {
	public:
		void initialize();

		void resizeInitialize( );

		void initFrame();
		void renderFrame();
		void render2D();

		void drawBox( float x, float y, float width, float height, Pixel3Byte color );
		void drawImage( Image image, Vec2d pos, bool flip_y = false, float scale = 1.0f, float opacity = 1.0f );
		
		void findTextureExtension( char *strFileName );

		PPUDraw ppuDraw;

		void setRes( int x, int y ) { xres = x; yres = y; }
		int getxRes() { return xres; }
		int getyRes() { return yres; }
	
		void setXYRes( int x, int y ) { xres = x; yres = y; }

		Renderer();
		virtual ~Renderer();
	private:

		int xres, yres;
		char *getGLErrorString( int error );

		//gl error code
		int glError;
	};
}
#endif // !defined( AFX_RENDERER_H__CDCC853B_3C74_4A4E_9339_2D8A8A00C624__INCLUDED_ )
