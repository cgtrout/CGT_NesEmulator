#include "precompiled.h"

#include "GLGeneral.h"

using namespace GUISystem;

//turns off safe string warning
#if _MSC_VER > 1000
#pragma warning( disable : 4996 )
#endif

Console::ConsoleVariable< int > timeProfileX (  
/*start val*/	550, 
/*name*/		"timeProfileX", 
/*description*/	"Sets x dim of TimeProfile window",
/*save?*/		SAVE_TO_FILE );

Console::ConsoleVariable< int > timeProfileY (  
/*start val*/	250, 
/*name*/		"timeProfileY", 
/*description*/	"Sets Y dim of TimeProfile window",
/*save?*/		SAVE_TO_FILE );
	
Console::ConsoleVariable< bool > showTimeProfile (  
/*start val*/	true, 
/*name*/		"showTimeProfile", 
/*description*/	"Set to true to show time profile",
/*save?*/		SAVE_TO_FILE );


GuiTimeProfiler::GuiTimeProfiler() {
	
}

GuiTimeProfiler::~GuiTimeProfiler( ) {
	_log->Write( "~GuiTimeProfiler" );
}

//GuiTimeProfiler::GuiTimeProfiler( std::string guitextures ) {
//	initialize( guitextures );
//}

void GuiTimeProfiler::initialize( ) {
	initialize( "gui/Console/Console.gt" );

	using namespace FrontEnd;
	SystemMain::getInstance( )->consoleSystem.variables.addBoolVariable( &showTimeProfile );
	SystemMain::getInstance( )->consoleSystem.variables.addIntVariable( &timeProfileX );
	SystemMain::getInstance( )->consoleSystem.variables.addIntVariable( &timeProfileY );
}

void GuiTimeProfiler::initialize( std::string guitextures ) {
	GUIElement::initialize( guitextures );
	//type = GE_Console;
	
	font.loadFont( "fonts/8x16.bmp" );
		
	try {
		background.image = loadImage( gt.getNextTexture() );
		background.image = convertToAlpha( 0,0,0,background.image );
		createTexture( background.image, &background.imageid );
	} catch( ImageException ) {
		throw GUIElementInitializeException( "GuiTimeProfiler:", "error loading image" );
	}

	lines.setOpen( true );
	addChild( &lines );
}


void GuiTimeProfiler::setX( GuiDim val ) {
	x = val;
	lines.setX( x );
}

void GuiTimeProfiler::setY( GuiDim val ) {
	y = val;
	lines.setY( y );
}


void GuiTimeProfiler::onLeftMouseDown() {
	GUIElement::onLeftMouseDown();	
}

void GuiTimeProfiler::onLeftMouseRelease() {
	GUIElement::onLeftMouseRelease();	
}

void GuiTimeProfiler::onMouseOver() {

}

void GuiTimeProfiler::onRender() {
	drawList.clear();

	if( showTimeProfile ) {
		background.x = this->x;
		background.y = this->y;
		background.stretchFactorx = width;
		background.stretchFactory = height;
		background.stretchType = GUISystem::ST_XY;
			
		//drawList.push_back( &background );
	}
}

void GuiTimeProfiler::onRightMouseDown() {

}

void GuiTimeProfiler::onRightMouseRelease() {

}

void GuiTimeProfiler::update() {
	if( showTimeProfile )
		lines.fillLines( reportString );
	else 
		lines.fillLines( " " );

	setX( (GuiDim)timeProfileX );
	setY( (GuiDim)timeProfileY );
}

void GuiTimeProfiler::onKeyDown( unsigned char key ) {
	
}

void GuiTimeProfiler::setReportString( const std::string &val ) {
	reportString = val;
}