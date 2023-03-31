#pragma once

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

//x and y screen dimensions
//#define NTSC_X 256
//#define NTSC_Y 240

#include "CGTSingleton.h"
using namespace CGTSingleton;

#include "NesPPU.h"
#include "NesPpuTools.h"

namespace FrontEnd {
	class SystemMain;
}

namespace NesEmulator {
	class NesMemory;
}

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
		Image& drawPatternTableF( NesEmulator::NesMemory* nesMemory );

		//draws raw pixel data to screen RGB pixel data expected
		//F added to end to aviod name collision with console vars
		void drawOutput( ubyte *data );

		Image& drawPaletteTableF( NesEmulator::NesMemory* nesMemory, NesEmulator::NesPalette *p );
	private:
		NesEmulator::NesPPUPixelGen ppuPixelGen;
		NesEmulator::NesPPUPaletteGen paletteGen;
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
		
		PPUDraw ppuDraw;

		void setRes( int x, int y ) { xres = x; yres = y; }
		float getxRes() { return static_cast<float>(xres); }
		float getyRes() { return static_cast<float>(yres); }
	
		void setXYRes( int x, int y ) { xres = x; yres = y; }

		Renderer( FrontEnd::SystemMain* );
		virtual ~Renderer();

		//state for drawing 'big' text
		struct BigText {
			std::string bigText;
    		float timer = 0.0f;
    		float fadeDuration = 2;
		} bigText;

		void setBigText( std::string_view newBigText );
		void renderBigText();

	private:

		int xres, yres;
		char *getGLErrorString( int error );

		FrontEnd::SystemMain* systemMain;
	};
}
