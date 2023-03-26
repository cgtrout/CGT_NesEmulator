// Renderer.cpp: implementation of the Renderer class.
//
//////////////////////////////////////////////////////////////////////
#include "precompiled.h"

#include "GLRenderer.h"
#include "SystemMain.h"

#include "NesPPU.h"
#include "Console.h"
#include "NesPalette.h"

#include "GLGeneral.h"

#include "ImGui/imgui.h"
#include "imgui/imgui_impl_opengl2.h"

extern FrontEnd::CLog *_log;

using namespace Console;
using namespace Render;

ConsoleVariable< bool > drawDebugPPU( 
/*start val*/	true, 
/*name*/		"drawDebugPPU", 
/*description*/	"Sets whether the debug pattern table should be drawn or not",
/*save?*/		SAVE_TO_FILE );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Renderer::Renderer( FrontEnd::SystemMain* sysMain ) :
	xres(0),
	yres(0),
	systemMain( sysMain )
{
	 
}

Renderer::~Renderer() {
}

float zdrawPos	= 0.0f;

void Renderer::initialize() {
	consoleSystem = &FrontEnd::SystemMain::getInstance( )->consoleSystem;
	consoleSystem->variables.addBoolVariable( &drawDebugPPU );

	resizeInitialize( );

	ppuDraw.initialize( );
}

/*
==============================================
Renderer::resizeInitialize()

  initialize after resize
==============================================
*/
void Renderer::resizeInitialize( ) {
	//prepare model and projection view for 2d drawing
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	glOrtho( 0.0, ( GLfloat )xres, 0.0, ( GLfloat )yres, -2, 1);
	glViewport( 0, 0, xres, yres );
}


/*
==============================================
Renderer::initFrame()

  initializes a frame
==============================================
*/
void Renderer::initFrame() {
	// Clear The Screen And The Depth Buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	zdrawPos = 0.0f;
}

/*
==============================================
Renderer::renderFrame()
==============================================
*/
void Renderer::renderFrame() {
	//disable multitexuring so it doesn't affect 2d drawing
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );

	render2D();
	
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	glEnable( GL_BLEND );

#ifdef _DEBUG
	//see if there were any errors this frame
	int glError = glGetError();
	static int numErrors;
	numErrors = 0;
	if( glError != GL_NO_ERROR ) {
		while( glError != GL_NO_ERROR ) {
			_log->Write( "gl error: %s", getGLErrorString( glError ) );//, systemMain->getCurrTime() );
			glError = glGetError();
			numErrors++;
		}		
		if( numErrors > 1 ) {
			_log->Write( "" );
		}
	}
	
#endif
}

/*
==============================================
Renderer::render2D()

  Render 2d aspects of the scene
==============================================
*/
void Renderer::render2D() {
	if( systemMain->nesMain.getState() == NesEmulator::States::Emulating ) {
		if( drawDebugPPU ) {
			systemMain->timeProfiler.startSection( "drawPPUDebug" );
			Image& patternImage = ppuDraw.drawPatternTableF( &systemMain->nesMain.nesMemory );   
			Image& paletteImage = ppuDraw.drawPaletteTableF( &systemMain->nesMain.nesMemory, &systemMain->nesMain.nesPpu.nesPalette);
			
			ImGui::Begin( "PPU Debug Window", drawDebugPPU.getPointer( ), 0 );
			ImGui::Image( ( void* )patternImage.handle, ImVec2( patternImage.getSizeX(), patternImage.getSizeY( ) ) );
			ImGui::Image( ( void* )paletteImage.handle, ImVec2( paletteImage.getSizeX( ), paletteImage.getSizeY( ) ) );
			ImGui::End( );
		}
	}

	//render nes output
	systemMain->timeProfiler.startSection( "drawPPU_main" );
	ppuDraw.drawOutput( systemMain->nesMain.nesPpu.vidoutBuffer );

	systemMain->nesMain.nesDebugger.draw( );

	systemMain->timeProfiler.startSection( "ImGui render" );
	ImGui::Render( );

	systemMain->timeProfiler.startSection( "ImGui drawdata" );
	ImGui_ImplOpenGL2_RenderDrawData( ImGui::GetDrawData( ) );

	//renderer.drawImage( nesMain.nesApu.getGraph(), &Vec2d( 0, 20 ),true, 1, 0.5f );
	systemMain->timeProfiler.startSection( "RenGUI" );
	systemMain->gui.render( );
}

void Renderer::drawBox( float x, float y, float width, float height, Pixel3Byte color ) {
	glDisable( GL_TEXTURE_2D );
	
	glBegin( GL_LINE_STRIP );
		glColor4f( color.color[ COLOR_RED ], color.color[ COLOR_GREEN ], color.color[ COLOR_BLUE ], 255 );     
		glVertex3f( x, y, 0 );
		glVertex3f( x + width, y, 0 );
		glVertex3f( x, y + height, 0 );
		glVertex3f( x + width, y + height, 0 );
	glEnd();

	glEnable( GL_TEXTURE_2D );
}

void Renderer::drawImage( Image image, Vec2d pos, bool flip_y, float scale, float opacity ) {
	glEnable( GL_BLEND );
	
	image.createGLTexture( );
    
	//glBindTexture( GL_TEXTURE_2D, imgid );

	float ystart;
	float yend;

	if( flip_y ) {
		ystart  = 1.0f;
		yend	= 0.0f;
	} else {
		ystart  = 0.0f;
		yend	= 1.0f;
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f( 1.0f, 1.0f, 1.0f, opacity );			

	    glBegin( GL_POLYGON );  
		glTexCoord2f( 0.0, ystart ); glVertex3f( pos.getX(), pos.getY(), zdrawPos );
		glTexCoord2f( 0.0, yend ); glVertex3f( pos.getX(), pos.getY() + ( image.sizeY*scale ), zdrawPos );
		glTexCoord2f( 1.0, yend ); glVertex3f( pos.getX() + ( image.sizeX*scale ), pos.getY() + ( image.sizeY*scale ), zdrawPos );
		glTexCoord2f( 1.0, ystart ); glVertex3f( pos.getX() + ( image.sizeX*scale ), pos.getY(), zdrawPos );
	glEnd();

	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f);			
	glEnable( GL_BLEND );

	zdrawPos += 0.01f;
}
/*
==============================================
PPUDraw::drawPatternTableF()

  Draws patterntable contents ( for debugging )

==============================================
*/
//positions of patterntable debug output
ConsoleVariable< bool > drawForceAspectRatio( 
/*start val*/	false, 
/*name*/		"drawForceAspectRatio", 
/*description*/	"Force 1 to 1 aspect ratio of main video output",
/*save?*/		SAVE_TO_FILE );


PPUDraw::PPUDraw() {
	
}

void PPUDraw::initialize( ) {
	consoleSystem = &FrontEnd::SystemMain::getInstance( )->consoleSystem;

	consoleSystem->variables.addBoolVariable( &drawForceAspectRatio );
}

//TODO create generic routine for drawing data buffer to screen
Image& PPUDraw::drawPatternTableF( NesEmulator::NesMemory *nesMemory ) {
	static Image img;
	
	//TODO check to see if game is actually loaded
    ppuPixelGen.genPatternTablePixelData( nesMemory );
    
    //TODO - better color implementation ( not hardcoded - based on pallette data )
    Pixel3Byte colors[ 4 ];
    colors[ 0 ].color[ COLOR_RED ] = 50;
    colors[ 0 ].color[ COLOR_GREEN ] = 50;
    colors[ 0 ].color[ COLOR_BLUE ] = 50;
    
    colors[ 1 ].color[ COLOR_RED ] = 100;
    colors[ 1 ].color[ COLOR_GREEN ] = 100;
    colors[ 1 ].color[ COLOR_BLUE ] = 0;
    
    colors[ 2 ].color[ COLOR_RED ] = 175;
    colors[ 2 ].color[ COLOR_GREEN ] = 175;
    colors[ 2 ].color[ COLOR_BLUE ] = 175;
    
    colors[ 3 ].color[ COLOR_RED ] = 254;
    colors[ 3 ].color[ COLOR_GREEN ] = 254;
    colors[ 3 ].color[ COLOR_BLUE ] = 254;
    ppuPixelGen.genPatternTablePixelData24Bit( colors );
    
    //feed data to gl
    //create texture

    img.channels = 3; 
    img.sizeX = ( 0x0f * 8 ) * 2;
    img.sizeY = 0x0f * 8;
	img.setData( ppuPixelGen.getPatternTablePixelData24Bit( ) );

	glDisable( GL_BLEND );
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );			

	img.createGLTexture( );
    glBindTexture( GL_TEXTURE_2D, img.handle );
   
	return img;
}

/* 
==============================================
void PPUDraw::drawOutput( ubyte *data )
==============================================
*/
void PPUDraw::drawOutput( ubyte* data ) {
	static Image img;

	img.channels = 3;
	img.sizeX = 256;
	img.sizeY = 256;
	img.setData( data );
	img.createGLTexture( );

	img.bindGLTexture( );

	ImGui::Begin( "Main Video", nullptr, ImGuiWindowFlags_NoScrollbar );
	auto width = ImGui::GetWindowWidth( );
	auto height = ImGui::GetWindowHeight( );

	if ( drawForceAspectRatio ) {
		float widthHighestPower = static_cast<float>(pow( 2, floor( log2( width ) ) ));
		float  heightHighestPower = static_cast<float>( pow( 2, floor( log2( height ) ) ));
		auto lowestVal = widthHighestPower < heightHighestPower ? widthHighestPower : heightHighestPower;

		ImGui::Image( ( void* )img.handle, ImVec2( lowestVal, lowestVal ) );
	} else {
		ImGui::Image( ( void* )img.handle, ImVec2( width, height ) );
	}
	ImGui::End( );
}

/*
==============================================
PpuDraw::drawPalletteTableF
==============================================
*/
Image& PPUDraw::drawPaletteTableF( NesEmulator::NesMemory* nesMemory, NesEmulator::NesPalette *pal ) {
	static Image img;

	paletteGen.genPalettePixelData( nesMemory, pal );

    img.channels = 3; 
    img.sizeX	 = 128;
    img.sizeY	 = 16;
    img.setData(paletteGen.getPixelData());
	img.createGLTexture( );
    
	//glDisable( GL_BLEND );
	//glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );			

	return img;
}

/* 
==============================================
char *Renderer::getGLErrorString( int error )
==============================================
*/
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

