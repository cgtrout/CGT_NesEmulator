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

extern FrontEnd::CLog *_log;

using namespace Console;
using namespace Render;

ConsoleVariable< bool > drawPatternTable( 
/*start val*/	true, 
/*name*/		"drawPatternTable", 
/*description*/	"Sets whether the debug pattern table should be drawn or not",
/*save?*/		SAVE_TO_FILE );

ConsoleVariable< bool > drawPaletteTable( 
/*start val*/	true, 
/*name*/		"drawPaletteTable",			
/*description*/	"Sets whether the debug palette table should be drawn or not",
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
	consoleSystem->variables.addBoolVariable( &drawPatternTable );
	consoleSystem->variables.addBoolVariable( &drawPaletteTable );

	//prepare model and projection view for 2d drawing
	glMatrixMode( GL_PROJECTION );
	//glPushMatrix();
	glLoadIdentity();
	//glOrtho( 0.0, ( GLfloat ) 640, 0.0, ( GLfloat ) 480, -2, 1 );
	glOrtho( 0.0, ( GLfloat ) xres, 0.0, ( GLfloat ) yres, -2, 1 );
	
	glMatrixMode( GL_MODELVIEW );
	//glPushMatrix();
	glLoadIdentity();
		
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
	glEnable( GL_TEXTURE_2D );
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
	
	//ensure correct matrix mode is selected for next frame
	glMatrixMode( GL_MODELVIEW );

	// Reset The matrix
	glLoadIdentity();	

	zdrawPos = 0.0f;
}

/*
==============================================
Renderer::renderFrame()
==============================================
*/
void Renderer::renderFrame() {
	render2D();
	
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
	//disable multitexuring so it doesn't affect 2d drawing
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	
	//TODO disable fragments
	//glDisable( GL_TEXTURE_2D );
	
	if( systemMain->nesMain.getState() == Emulating ) {
		if( drawPatternTable.getValue() == 1.0f ) {
			ppuDraw.drawPatternTable();   
		}
		if( drawPaletteTable.getValue() == 1.0f ) {
			ppuDraw.drawPaletteTable( &FrontEnd::SystemMain::getInstance()->nesMain.nesPpu.nesPalette );
		}
	}
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
	static GLuint imgid = 0;
				
	if( imgid != 0 ) {
        glDeleteTextures( 1, &imgid );
    }

	glEnable( GL_BLEND );
	
    if( image.channels == 3 ) {
		createTexture( image, &imgid, GL_RGB );
	} else {
		createTexture( image, &imgid, GL_RGBA );
	}
    
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
PPUDraw::drawPatternTable()

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
void PPUDraw::drawPatternTable() {
    static GLuint imgid = 0;
    
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
    
    Image img;

    if( imgid != 0 ) {
        glDeleteTextures( 1, &imgid );
    }

    img.channels = 3; 
    img.sizeX = ( 0x0f * 8 ) * 2;
    img.sizeY = 0x0f * 8;
	img.setData( ppuPixelGen.getPatternTablePixelData24Bit( ) );

	glDisable( GL_BLEND );
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );			

    createTexture( img, &imgid, GL_RGB );
    glBindTexture( GL_TEXTURE_2D, imgid );
    
    float zdrawpos = 0.0001f;

    glBegin( GL_POLYGON );  
		glTexCoord2f( 0.0, 1.0 ); glVertex3f( x, y, zdrawpos );
		glTexCoord2f( 0.0, 0.0 ); glVertex3f( x, y + ( img.sizeY*scale ), zdrawpos );
		glTexCoord2f( 1.0, 0.0 ); glVertex3f( x + ( img.sizeX*scale ), y + ( img.sizeY*scale ), zdrawpos );
		glTexCoord2f( 1.0, 1.0 ); glVertex3f( x + ( img.sizeX*scale ), y, zdrawpos );
	glEnd();

	glEnable( GL_BLEND );
	//img.data = 0;
}

/* 
==============================================
void PPUDraw::drawOutput( ubyte *data )
==============================================
*/
void PPUDraw::drawOutput( ubyte *data ) {
	//is it correct to arbitrarily set this to 1?
	static GLuint imgid = 0;
	Image img;

	float x = outputX.getValue();
	float y = outputY.getValue();
	
	float scale		= outputScale.getValue();	
    
    if( imgid != 0 ) {
        glDeleteTextures( 1, &imgid );
    }
	
	//TODO fix sketchy argument pass of image
    img.channels = 3; 
    img.sizeX	 = 256; 
    img.sizeY	 = 256;
    img.setData(data);
    
	Vec2d pos( x, y );
	systemMain->renderer.drawImage( img, pos, true, scale );
}

void PPUDraw::drawPaletteTable( PpuSystem::NesPalette *pal ) {
	//is it correct to arbitrarily set this to 1?
	static GLuint imgid = 0;
	Image img;

	float x = paletteTableX;
	float y = paletteTableY;
	
	float scale		= paletteTableScale;
    
    if( imgid != 0 ) {
        glDeleteTextures( 1, &imgid );
    }

	paletteGen.genPalettePixelData( pal );

    img.channels = 3; 
    img.sizeX	 = 128;
    img.sizeY	 = 16;
    img.setData(paletteGen.getPixelData());
    
	glDisable( GL_BLEND );
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );			

    createTexture( img, &imgid, GL_RGB );
    glBindTexture( GL_TEXTURE_2D, imgid );
    
    float zdrawpos	= 0.0f;

    glBegin( GL_POLYGON );  
		glTexCoord2f( 0.0, 0.0 ); glVertex3f( x, y, zdrawpos );
		glTexCoord2f( 0.0, 1.0 ); glVertex3f( x, y + ( img.sizeY*scale ), zdrawpos );
		glTexCoord2f( 1.0, 1.0 ); glVertex3f( x + ( img.sizeX*scale ), y + ( img.sizeY*scale ), zdrawpos );
		glTexCoord2f( 1.0, 0.0 ); glVertex3f( x + ( img.sizeX*scale ), y, zdrawpos );
	glEnd();

	glEnable( GL_BLEND );
	
	//reset data pointer so data isn't deleted
	//img.data = 0;
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

