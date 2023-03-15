// Renderer.cpp: implementation of the Renderer class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning( disable : 4786 )

#include "precompiled.h"

#include "GLRenderer.h"
#include "SystemMain.h"

#include "NesPPU.h"
#include "Console.h"
#include "NesPalette.h"

#include "GLGeneral.h"

#include "ImGui/imgui.h"

extern FrontEnd::CLog *_log;

using namespace Console;
using namespace Render;

ConsoleVariable< bool > drawDebugPPU( 
/*start val*/	true, 
/*name*/		"drawDebugPPU", 
/*description*/	"Sets whether the debug pattern table should be drawn or not",
/*save?*/		SAVE_TO_FILE );

//TODO this turns off the conversion ( From int to float ) warning
//this should be fixed correctly
#if _MSC_VER > 1000
#pragma warning( disable : 4244 )
#endif

//turn off unsafe string function warnings
//TODO change these someday...
#if _MSC_VER > 1000
#pragma warning( disable : 4996 )
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Renderer::Renderer() {
	 
}


Renderer::~Renderer() {
}

float zdrawPos	= 0.0f;

void Renderer::initialize() {
	Console::ConsoleSystem* consoleSystem = &FrontEnd::SystemMain::getInstance( )->consoleSystem;
	consoleSystem->variables.addBoolVariable( &drawDebugPPU );

	resizeInitialize( );
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
	if( systemMain->nesMain.getState() == Emulating ) {
		if( drawDebugPPU ) {
			Image& patternImage = ppuDraw.drawPatternTableF();   
			Image& paletteImage = ppuDraw.drawPaletteTableF(&systemMain->nesMain.nesPpu.nesPalette);
			
			ImGui::Begin( "PPU Debug Window", drawDebugPPU.getPointer( ), 0 );
			ImGui::Image( ( void* )patternImage.handle, ImVec2( patternImage.sizeX, patternImage.sizeY ) );
			ImGui::Image( ( void* )paletteImage.handle, ImVec2( paletteImage.sizeX, paletteImage.sizeY ) );
			ImGui::End( );
		}
	}

	//render nes output
	systemMain->timeProfiler.startSection( "RenPPU" );
	ppuDraw.drawOutput( systemMain->nesMain.nesPpu.vidoutBuffer );
	systemMain->timeProfiler.stopSection( "RenPPU" );
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
		glTexCoord2f( 0.0, ystart ); glVertex3f( pos.x, pos.y, zdrawPos );
		glTexCoord2f( 0.0, yend ); glVertex3f( pos.x, pos.y + ( image.sizeY*scale ), zdrawPos );
		glTexCoord2f( 1.0, yend ); glVertex3f( pos.x + ( image.sizeX*scale ), pos.y + ( image.sizeY*scale ), zdrawPos );
		glTexCoord2f( 1.0, ystart ); glVertex3f( pos.x + ( image.sizeX*scale ), pos.y, zdrawPos );
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
ConsoleVariable< int > patternTableX( 
/*start val*/	535, 
/*name*/		"patternTableX", 
/*description*/	"x position of debug pattern table",
/*save?*/		SAVE_TO_FILE );

ConsoleVariable< int > patternTableY( 
/*start val*/	400, 
/*name*/		"patternTableY", 
/*description*/	"y position of debug pattern table",
/*save?*/		SAVE_TO_FILE );

ConsoleVariable< float > patternTableScale( 
/*start val*/	1.0f, 
/*name*/		"patternTableScale", 
/*description*/	"scaling debug pattern table",
/*save?*/		SAVE_TO_FILE );

ConsoleVariable< int > outputX ( 
/*start val*/	20, 
/*name*/		"outputX", 
/*description*/	"x position of main render output",
/*save?*/		SAVE_TO_FILE );

ConsoleVariable< int > outputY ( 
/*start val*/	-9, 
/*name*/		"outputY", 
/*description*/	"y position of main render output",
/*save?*/		SAVE_TO_FILE );

ConsoleVariable< float > outputScale( 
/*start val*/	2.0f, 
/*name*/		"outputScale", 
/*description*/	"scaling of main render output",
/*save?*/		SAVE_TO_FILE );

ConsoleVariable< int > paletteTableX( 
/*start val*/	604, 
/*name*/		"paletteTableX", 
/*description*/	"x position of debug palette output",
/*save?*/		SAVE_TO_FILE );

ConsoleVariable< int > paletteTableY( 
/*start val*/	65, 
/*name*/		"paletteTableY", 
/*description*/	"y position of debug palette output",
/*save?*/		SAVE_TO_FILE );

ConsoleVariable< float > paletteTableScale( 
/*start val*/	1.0f, 
/*name*/		"paletteTableScale", 
/*description*/	"scaling of debug palette output",
/*save?*/		SAVE_TO_FILE );

PPUDraw::PPUDraw() {
	
}

void PPUDraw::initialize( ) {
	Console::ConsoleSystem* consoleSystem = &FrontEnd::SystemMain::getInstance( )->consoleSystem;

	consoleSystem->variables.addIntVariable( &patternTableX );
	consoleSystem->variables.addIntVariable( &patternTableY );
	consoleSystem->variables.addFloatVariable( &patternTableScale );

	consoleSystem->variables.addIntVariable( &outputX );
	consoleSystem->variables.addIntVariable( &outputY );
	consoleSystem->variables.addFloatVariable( &outputScale );

	consoleSystem->variables.addIntVariable( &paletteTableX );
	consoleSystem->variables.addIntVariable( &paletteTableY );
	consoleSystem->variables.addFloatVariable( &paletteTableScale );
}

//TODO create generic routine for drawing data buffer to screen
Image& PPUDraw::drawPatternTableF() {
	static Image img;
	
	float x = patternTableX.getValue();
	float y = patternTableY.getValue();
	float scale = patternTableScale.getValue();
    
	//TODO check to see if game is actually loaded
    ppuPixelGen.genPatternTablePixelData();
    
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
void PPUDraw::drawOutput( ubyte *data ) {
	static Image img;

	float x = outputX.getValue();
	float y = outputY.getValue();
	
	float scale		= outputScale.getValue();	
    
	//TODO fix sketchy argument pass of image
    img.channels = 3; 
    img.sizeX	 = 256; 
    img.sizeY	 = 256;
    img.setData(data);
	img.createGLTexture( );

	img.bindGLTexture( );

	ImGui::Begin( "Main Video", nullptr, 0 );
		ImGui::Image( ( void* )img.handle, ImVec2( img.sizeX*2, img.sizeY*2 ) );
	ImGui::End( );
}

/*
==============================================
PpuDraw::drawPalletteTableF
==============================================
*/
Image& PPUDraw::drawPaletteTableF( PpuSystem::NesPalette *pal ) {
	static Image img;

	float x = paletteTableX;
	float y = paletteTableY;
	
	float scale		= paletteTableScale;
    
	paletteGen.genPalettePixelData( pal );

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
findTextureExtension()

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

