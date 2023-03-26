#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "precompiled.h"

#include "GUIConsole.h"
#include "Input.h"

using namespace GUISystem;

#include <SDL_scancode.h>

GUIConsole::GUIConsole(FrontEnd::InputSystem::Input* inputSystem) : 
	offset( 0 )
{
	this->inputSystem = inputSystem;
	initialize( "gui/console/console.gt" );
	//initialize( "C:\\Users\\cgtro\\Documents\\2023 Repos\\Nes_Emulator\\gui\\console\\console.gt" );
}

GUIConsole::GUIConsole( std::string guitextures ) {
	initialize( guitextures );
}

void GUIConsole::initialize( std::string guitextures ) {
	GUIElement::initialize( guitextures );
	//type = GE_Console;
	
	font.loadFont( "fonts/8x16.bmp" );

	width = 500;
	height = 198;
	
	x = 0;
	y = 0;

	try {
		background.image = loadImage( gt.getNextTexture() );
		background.image.createGLTexture( );
	}
	catch( ImageException ) {
		throw GUIElementInitializeException( "GUIConsole:", "error loading image" );
	}
	
	editLine.setWidth( width );
	editLine.setInputSystem( inputSystem );
	
	addChild( &editLine );	
	
	//set up all the lines of the console
	int ly;
	for( ly = 0; ly < CONSOLE_LINES; ly++ )
	 {
		lines[ ly ].loadFont( &font );
		lines[ ly ].setString( "" );
		lines[ ly ].setWidth( width );
		lines[ ly ].setX( 3 + x );
		//lines[ ly ].setY( 3 + ( ly * lines[ ly ].getFont()->getFontHeight() + 2 ) );
		lines[ ly ].setY( y + 3 + ( ly * lines[ ly ].getFont()->getFontHeight() + 2 ) );
		lines[ ly ].setOpen( true );
		addChild( &lines[ ly ] );
	}
	++ly;
	editLine.setX( x );
	editLine.setY( y + ( ly * font.getFontHeight() + 2 ) );
	editLine.clearText();
	changed = true;
}

GUIConsole::~GUIConsole() {
	//todo free elements
}

void GUIConsole::onLeftMouseDown() {
	GUIElement::onLeftMouseDown();	
}

void GUIConsole::onLeftMouseRelease() {
	GUIElement::onLeftMouseRelease();	
}

void GUIConsole::onMouseOver() {

}

void GUIConsole::onRender() {
	std::string history;
	
	//performance optimization
	//if the output can not have changed then their
	//is no reason to go through all these lines again
	if( changed ) {
		std::string temp = consoleSystem->getHistoryLines( CONSOLE_LINES, offset );
		
		if( !temp.empty() ) {
			history = temp;

			//go through history string 
			//lines, one by one, until they are all filled up
			int pos = 0;//pos in history
			int cline = 0;//CONSOLE_LINES-1;//current line
			int lastPos = history.length();
			int endOfCurrentLine;
			for( ; pos < lastPos; cline++ ) {
				endOfCurrentLine = history.find( "\n" , pos );
				std::string _substr = history.substr( pos, endOfCurrentLine - pos );
				lines[ cline ].setString( _substr );
				pos = endOfCurrentLine + 1;
			}
			drawList.clear();

			background.x = this->x;
			background.y = this->y;
			background.stretchFactorx = width;
			background.stretchFactory = height;
			background.stretchType = GUISystem::ST_XY;
		}
			
		drawList.push_back( &background );
		changed = false;
	}
}

void GUIConsole::onRightMouseDown() {

}

void GUIConsole::onRightMouseRelease() {

}

void GUIConsole::update() {
}

void GUIConsole::ConsoleEditBox::onEnterKey() {
	consoleSystem->executeRequest( this->text, true );
	( ( GUIConsole* )parent )->changed = true;
	clearText();	
}

void GUIConsole::onKeyDown( SDL_Keycode key ) {
	std::string res;
	
	int origOffset = offset;
	
	switch( key ) {
	case SDLK_PAGEUP:
		//move up a line
		offset++;
		//offset can not be larger than number of total lines in history
		if( offset >= consoleSystem->sizeOfHistory() ) {
			offset--;
		}
		break;
	case SDLK_PAGEDOWN:
		//move down a line
		offset--;
		if( offset < 0 ) {
			offset = 0;
		}
		break;
	case SDLK_TAB:
		res = consoleSystem->printMatches( editLine.getText() );
		if( !res.empty() ) {
			editLine.setText( res );
		}
		changed = true;
		break;
	case SDLK_UP:
		//get previous request
		editLine.setText( consoleSystem->getPreviousRequest() );
		break;
	case SDLK_DOWN:
		//get next request
		editLine.setText( consoleSystem->getNextRequest() );
		break;
	}

	if( origOffset = offset ) {
		changed = true;
	}
}

void GUIConsole::setOpen( bool val ) {
	GUIElement::setOpen( val );
	using namespace FrontEnd;
	using namespace InputSystem;
}

/*
void GUIConsole::setAsActiveElement() {
	GUIElement::setAsActiveElement();

	editLine.setAsActiveElement();
}*/
