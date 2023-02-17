#include "precompiled.h"

#include "GLGeneral.h"

using namespace GUISystem;

//turns off safe string warning
#if _MSC_VER > 1000
#pragma warning( disable : 4996 )
#endif


GUIFPSCounter::GUIFPSCounter() {
	initialize( "gui/Console/Console.gt" );
}

GUIFPSCounter::GUIFPSCounter( std::string guitextures ) {
	initialize( guitextures );
}

void GUIFPSCounter::initialize( std::string guitextures ) {
	GUIElement::initialize( guitextures );
	//type = GE_Console;
	
	font.loadFont( "fonts/8x16.bmp" );
	//font.loadFont( "fonts/8x16.bmp" );
	fpsCounter.loadFont( &font );
	
	try {
		background.image = loadImage( gt.getNextTexture() );
		background.image = convertToAlpha( 0,0,0,background.image );
		createTexture( background.image, &background.imageid );
	}
	catch( ImageException ) {
		throw GUIElementInitializeException( "GUIFPSCounter:", "error loading image" );
	}
	
	//fpsCounter.setWidth( getWidth() );
	
	addChild( &fpsCounter );	
	width = font.getFontWidth() * 11;
	height = font.getFontHeight();
	
	fpsCounter.setX( x );
	fpsCounter.setY( y );
	fpsCounter.setWidth( width );
	fpsCounter.setString( "xxx fps" );
	fpsCounter.setOpen( true );
}

GUIFPSCounter::~GUIFPSCounter() {
	
}

void GUIFPSCounter::setX( GuiDim val ) {
	x = val;
	fpsCounter.setX( x );
}

void GUIFPSCounter::setY( GuiDim val ) {
	y = val;
	fpsCounter.setY( y );
}


void GUIFPSCounter::onLeftMouseDown() {
	GUIElement::onLeftMouseDown();	
}

void GUIFPSCounter::onLeftMouseRelease() {
	GUIElement::onLeftMouseRelease();	
}

void GUIFPSCounter::onMouseOver() {

}

void GUIFPSCounter::onRender() {
	drawList.clear();

	background.x = this->x;
	background.y = this->y;
	background.stretchFactorx = width;
	background.stretchFactory = height;
	background.stretchType = GUISystem::ST_XY;
		
	//drawList.push_back( &background );
}

void GUIFPSCounter::onRightMouseDown() {

}

void GUIFPSCounter::onRightMouseRelease() {

}

void GUIFPSCounter::update() {
	static char buffer[ 50 ];
	sprintf( buffer, "%.2f fps", fps );
	fpsCounter.setString( buffer );
}

void GUIFPSCounter::onKeyDown( unsigned char key ) {
	
}
