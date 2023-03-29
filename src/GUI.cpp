#include "precompiled.h"
#include <algorithm>
#include <functional>
#include <SDL_scancode.h>
#include "GUI.h"
#include "GLGeneral.h"

using namespace GUISystem;
using namespace FrontEnd;

using namespace Console;

ConsoleVariable< bool > drawDebugLines ( 
/*start val*/	false, 
/*name*/		"drawGuiDebugLines", 
/*description*/	"draws debug lines around gui objects for debugging",
/*save?*/		SAVE_TO_FILE );

ConsoleVariable< float > guiOpacity( 
/*start val*/	0.85f, 
/*name*/		"guiOpacity", 
/*description*/	"sets global opacity for gui system",
/*save?*/		SAVE_TO_FILE );

ConsoleVariable< float > fontOpacity( 
/*start val*/	1.0f, 
/*name*/		"fontOpacity", 
/*description*/	"sets font opacity for gui system",
/*save?*/		SAVE_TO_FILE );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GUI::GUI( FrontEnd::InputSystem::Input *inputSystem ) :
	inputSystem( inputSystem ),
	zdrawpos( 0.0f ),
	drawGUI( true ),
	usingMouse( true ),
	renderSystem()
{
}

GUI::~GUI() {
}

void GUI::initialize( ) {
	consoleSystem->variables.addBoolVariable( &drawDebugLines );
	consoleSystem->variables.addFloatVariable( &guiOpacity );
	consoleSystem->variables.addFloatVariable( &fontOpacity );
}

/*
==============================================
GUI::runFrame()

  updates all elements based on user's actions
==============================================
*/
void GUI::runFrame() {
	GUIElement *activeElem = findElementCursorOver();
	
	if( activeElem != nullptr && usingMouse ) {
		activeElem->onMouseOver();
		if( inputSystem->isMouseLeftDown() ) {
			unactivateAllElements();
			activeElem->onLeftMouseDown();
		}
		if( inputSystem->isMouseLeftUp() )
			activeElem->onLeftMouseRelease();
		if( inputSystem->isMouseRightDown() )
			activeElem->onRightMouseDown();
		if( inputSystem->isMouseRightUp() )
			activeElem->onRightMouseRelease();
	}
	else {
		if( inputSystem->isMouseLeftDown() ) 
			unactivateAllElements();
	}
	
	sendActiveToFront();
	//inputSystem->setState( Input::NORMAL_MODE );
	updateElementsList( elements );
}

/*
==============================================
GUI::updateElementsList()

  update all elements ( and its children ) in list 
  elems given as parameter
==============================================
*/
void GUI::updateElementsList(std::vector< GUIElement* > elems ) {
	using namespace FrontEnd::InputSystem;
	GUIElement *curr;
	std::vector< GUIElement* >::iterator iter;
	for( iter = elems.begin(); iter != elems.end(); iter++ ) {
		curr = ( GUIElement* )( *iter );
		if( !curr->isOpen() ) {
			continue;
		}
		if( inputSystem->getLastKeyPressed() != 0 && curr->isActiveElement() ) {
			curr->onKeyDown( inputSystem->getLastKeyPressed() );
		}
		if( curr->getType() == GE_EDITBOX ) {
			if( curr->isActiveElement() && usingMouse ) {
				inputSystem->setState( Input::InputSystemStates::TYPE_MODE );
				curr->update();
			}
			else {
				inputSystem->setState( Input::InputSystemStates::NORMAL_MODE );
				curr->update();
			}
		}
		else {	//not GE_EDITBOX
			curr->update();
		}
		
		if( !curr->children.empty() ) {
			updateElementsList( curr->children );
		}
	}
}

/*
==============================================
GUI::sendActiveToFront()

  sends active GUIElement to the front of the list
  so that it is drawn first
==============================================
*/
void GUI::sendActiveToFront() {
	GUIElement *elem;
	std::vector< GUIElement* > newList;

	std::vector< GUIElement* >::iterator iter;
	for( iter = elements.begin(); iter != elements.end(); iter++ ) {
		elem = ( GUIElement* )( *iter );
		
		if( elem->isActiveElement() ) {
			newList.push_back( elem );
			continue;
		}
		newList.push_back( elem );
	}
	elements = newList;
}

/*
==============================================
GUI::findElementCursorOver()

  finds the element that the mouse cursor is over
  and returns that element.

  returns nullptr if no element is found
==============================================
*/
GUIElement *GUI::findElementCursorOver() {
	std::vector< GUIElement* > currlist;
	GUIElement *currelem = nullptr;
	GUIElement *retelem = nullptr;

	int mousex = inputSystem->getMouseX();
	int mousey = inputSystem->getMouseY();

	currlist = elements;
			
	while( !currlist.empty() ) {
		std::vector< GUIElement* >::iterator iter;
		for( iter = currlist.begin(); iter != currlist.end(); iter++ ) {
			currelem = ( GUIElement* )( *iter );

			if ( currelem->isOpen()
			&&   mousex > currelem->getX() 
			&&   mousex < currelem->getX() + currelem->getWidth()
			&&   mousey > currelem->getY()
			&&   mousey < currelem->getY() + currelem->getHeight() ) {
					currlist = currelem->children;
					retelem = currelem;
					break;
			}
			currelem = nullptr;
		}
		if( currelem == nullptr )
			break;
	}
	return retelem;
}

/*
==============================================
GUI::unactivateAllElements()

  unactivates all elements in the elements list
==============================================
*/
void GUI::unactivateAllElements() {
	for ( auto e : elements ) {
		e->unactivateElement( );
	}
}

/*
==============================================
GUI::renderElements(std::vector< GUIElement* > elems )

  renders elements in elems list
==============================================
*/
void GUI::renderElements(std::vector< GUIElement* > elems ) {
	for( auto iter = elems.rbegin(); iter != elems.rend(); iter++ ) {
		GUIElement* curr = ( GUIElement* )( *iter );

		if( curr->isOpen() ) {			
			if( ( int )drawDebugLines.getValue() == TRUE ) {
				renderDebugLines( curr );
			}
			if( curr->getType() == GE_FONT ) {
				renderTextLabel( ( TextLabel* )curr );
			}
			else {
				curr->onRender();
				renderDrawList( curr->drawList );
			}			
			
			zdrawpos += 0.0001f;

			//if it has children draw them
			if( !curr->children.empty() ) {
				renderElements( curr->children );	
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////
// GUIElement Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GUIElement::GUIElement( std::string guitextures ) :
	inputSystem( nullptr )	//must be passed pointer later
{
}

GUIElement::~GUIElement() {
	_log->Write( "GUIElement destructor called type=%d", this->getType() );
}

void GUIElement::initialize( std::string guitextures )
 {
	x = 0; y = 0; width = 128; height = 128;
	type = GE_BASE;
	parent = nullptr;

	open = true;
	activeElement = false;

	try {
		gt.loadFile( guitextures );
	}
	catch( GUITextures::GUITexturesLoadException ) {
		throw GUIElementInitializeException( "GUIElement::initialize", "Error loading GUI Texture file" );
	}
	parent = nullptr;
}

/*
==============================================
GUIElement *GUIElement::getRootParent()
==============================================
*/
GUIElement *GUIElement::getRootParent() {
	GUIElement *ret = this;
	while( ret->parent != nullptr ) {
		ret = ret->parent;
	}
	
	return ret;
}

/*
==============================================
void GUIElement::move( int xm, int ym )

  moves element and all its children 
  
  xm- x movement
  ym- y movement
==============================================
*/
void GUIElement::move( int xm, int ym ) {
	x += xm;
	y += ym;

	std::vector< GUIElement* >::iterator iter;
	for( iter = children.begin(); iter != children.end(); iter++ ) 
		(*iter)->move( xm, ym );
}

/*
==============================================
void GUIElement::unactivateChildren(std::vector< GUIElement* > childList )
==============================================
*/
void GUIElement::unactivateChildren(std::vector< GUIElement* > childList ) {
	std::vector< GUIElement* >::iterator iter;
	GUIElement *curr;

	for( iter = childList.begin(); iter != childList.end(); iter++ ) {
		curr = ( GUIElement* )( *iter );
		curr->activeElement = false;
		if( !curr->children.empty() ) {
			unactivateChildren( curr->children );
		}
	}
}

/*
==============================================
GUIElement *GUIElement::getActiveChild(std::vector< GUIElement* > childList )
==============================================
*/
GUIElement *GUIElement::getActiveChild(std::vector< GUIElement* > childList ) {
	std::vector< GUIElement* >::iterator iter;
	GUIElement *curr = nullptr;

	for( iter = childList.begin(); iter != childList.end(); iter++ ) {
		curr = ( GUIElement* )( *iter );

		if( curr->activeElement ) {
			if( !curr->children.empty() )
				curr = getActiveChild( curr->children );
			break;
		}
	}
	return curr;
}

/*
=================================================================
=================================================================
GUITextures Class

  GUITexture files are used to define textures to load for GUI
  elements
=================================================================
=================================================================
*/
void GUITextures::loadFromFile( std::string_view fileNameIn ) {
	std::ifstream is( fileNameIn.data() );
	std::string error;
	this->fileName = fileNameIn;

	if( is.fail( ) ) {
		std::error_code ec( errno, std::generic_category( ) );
		std::stringstream ss;
		ss << "Error loading file \"" << fileNameIn << "\": " << ec.message( );
		throw GUITexturesLoadException( "GUITextures::loadFromFile", ss.str( ) );
	}

	for ( std::string line; std::getline( is, line ); )
	{
		parseLine( line );
	}
	
	is.close();
}

void GUITextures::parseLine( std::string_view line ) {
	unsigned int x = 0;	//position in line
	int firstq = -1;	//first quote
	int secq = -1;		//second quote
	char currchar;

	if( line.length() == 0 )
		return;
	
	//find first '"'
	for( ; x < line.length(); x++ ) {
		currchar = line[ x ];
		if( currchar == '"' ) {
			firstq = x++;
			break;
		}
	}
	if( firstq == -1 ) {
		return;
	}

	//find sec '"'
	for( ; x < line.length(); x++ ) {
		currchar = line[ x ];
		if( currchar == '"' ) {
			secq = x;
			break;
		}
	}
	if( secq == -1 ) {
		std::stringstream error;
		error	<< "File: \"" << fileName << "\" Line #"
				<< x << " is an invalid line - must have a "
				<< "file name contained within quotes/n";
		throw GUITexturesLoadException( "GuiTextures::parseLine", error.str() );
	}
	std::string newLine( line.substr( firstq + 1, secq - firstq - 1 ) );
	fileList.push_back( newLine );
}

/*
=================================================================
=================================================================
EditBox Class
=================================================================
=================================================================
*/
EditBox::EditBox() {
	initialize( "gui/EditBox/editbox.gt" );	
}

EditBox::EditBox( std::string guitextures ) {
	initialize( guitextures );
}

void EditBox::initialize( std::string guitextures ) {
	name = "EditBox";
	GUIElement::initialize( guitextures );
	type = GE_EDITBOX;
	height = 22;
	width = 100;
	multiLine = false;
	draw = true;

	showCursor = false;
	cursorPos = 0;
	viewPos = 0;

	blinkTimeWait = 0.4f;
	blinkTimeLast = 0;
	elapsedTime = 0;
	
	font.loadFont( "fonts/8x16.bmp" );
	
	text = "blank";
	boxtext.loadFont( &font );
	boxtext.setString( text.c_str() );

	addChild( &boxtext );

	try	 {
		l.image = (loadImage( gt.getNextTexture() ));
		r.image = (loadImage( gt.getNextTexture() ));
		b.image = (loadImage( gt.getNextTexture() ));
		t.image = (loadImage( gt.getNextTexture() ));
		bl.image = (loadImage( gt.getNextTexture() ));
		br.image = (loadImage( gt.getNextTexture() ));
		tl.image = (loadImage( gt.getNextTexture() ));
		tr.image = (loadImage( gt.getNextTexture() ));
		cursor.image = (loadImage( gt.getNextTexture() ));
		
	}
	catch( ImageException ) {
		throw GUIElementInitializeException( "EditBox", "loading image" );
	}
}

EditBox::~EditBox() {
}

void EditBox::onLeftMouseDown() {
	inputSystem->setUseDelay( true );
	//FIXME blink timer
	//blinkTimeLast = timer->getCurrTime();
	GUIElement::onLeftMouseDown();		
}

void EditBox::onLeftMouseRelease() {
	GUIElement::onLeftMouseRelease();
}

void EditBox::onMouseOver() {

}

void EditBox::onRightMouseDown() {
}

void EditBox::onRightMouseRelease() {
}

void EditBox::onEnterKey() {
}

void EditBox::onRender() {
	drawList.clear();

	if( draw ) {
		l.x = x;
		l.y = y + t.image.sizeY;
		l.stretchFactory = height - ( 2 * b.image.sizeY );
		l.stretchType = ST_Y;

		r.x = x + ( width - ( r.image.sizeX ) );
		r.y = y + t.image.sizeY;
		r.stretchFactory = height - ( 2 * b.image.sizeY );
		r.stretchType = ST_Y;

		bl.x = x;
		bl.y = y + ( height - bl.image.sizeY );
		bl.stretchType = ST_NONE;

		br.x = x + ( width - br.image.sizeX );
		br.y = y + ( height - bl.image.sizeY );
		br.stretchType = ST_NONE;

		b.x = x + ( bl.image.sizeX );
		b.y = y + ( height - bl.image.sizeY );
		b.stretchFactorx = width - ( 2 * br.image.sizeX );
		b.stretchType = ST_X;
		
		tl.x = x;
		tl.y = y;
		tl.stretchType = ST_NONE;
		
		t.x = x + tl.image.sizeX;
		t.y = y;
		t.stretchFactorx = width - ( 2 * tr.image.sizeX );
		t.stretchType = ST_X;

		tr.x = x + ( width - tr.image.sizeX );
		tr.y = y;
		tr.stretchType = ST_NONE;

		drawList.push_back(&l );
		drawList.push_back(&r );
		drawList.push_back(&bl );
		drawList.push_back(&br );
		drawList.push_back(&b );
		drawList.push_back(&tl );
		drawList.push_back(&t );
		drawList.push_back(&tr );
	}

	//control blinking of position cursor
	if( activeElement ) {
		//FIXME
		//elapsedTime += timer->getCurrTime();

		if( elapsedTime > blinkTimeWait ) {
			showCursor = !showCursor;
			elapsedTime -= blinkTimeWait;
			//blinkTimeLast = currTime;
		}	
	}
	else {
		showCursor = false;
	}

	if( showCursor ) {
		cursor.stretchType = ST_NONE;
		cursor.x = ( x + 4 + cursorPos * font.getFontWidth() );
		cursor.y = y + 4;

		drawList.push_back( &cursor );
	}

	boxtext.setX( x + 5 );
	boxtext.setY( y + 2 );
}

void EditBox::update() {
	if( activeElement ) {
		//max chars that can be fit in editbox at a time
		unsigned int maxSize = ( (int)width - 10 ) / (int)font.getFontWidth();

		if( inputSystem->isKeyDown( SDLK_RETURN ) ) {
			onEnterKey();
		}

		if( inputSystem->isKeyDown( SDLK_HOME ) ) {
			cursorPos = 0;
			viewPos = 0;
		}
		if( inputSystem->isKeyDown( SDLK_END ) ) {
			if( text.length() < maxSize ) {
				cursorPos = text.length();
				viewPos = 0;
			}
			else {
				cursorPos = maxSize;
				viewPos = text.length() - maxSize;
			}			                             
		}
		if( inputSystem->isKeyDown( SDLK_LEFT ) ) {
			if( viewPos + cursorPos > 0 )
				--cursorPos;
		}
		if( inputSystem->isKeyDown( SDLK_RIGHT ) ) {
			if( viewPos + cursorPos < text.length() )
				++cursorPos;
		}
		if( inputSystem->isKeyDown( SDLK_BACKSPACE ) ) {
			if( text.length() > 0 && viewPos + cursorPos != 0 ) {
				text.erase( viewPos + ( cursorPos-1 ), 1 );
				--cursorPos;
			}
			if( cursorPos < 0 ) {
				if( viewPos < maxSize ) {
					cursorPos = viewPos -1;
					viewPos = 0;
				}
				else {
					cursorPos = maxSize-1;
					viewPos -= maxSize;
				}	
			}
		}
		if( inputSystem->isKeyDown( SDLK_DELETE ) ) {
			if( text.length() > 0 ) {
				text.erase( viewPos + cursorPos, 1 );
			}
		}
		std::string strIns;
		
		//TODO get text input from stl
		strIns = inputSystem->getTextInput();
		inputSystem->clearTextInput( );

		unsigned int availSize = maxSize - text.length();
		unsigned int sizeToUse;
		if( strIns.size() > availSize ) {
			sizeToUse = availSize;
		}
		else {
			sizeToUse = strIns.size();
		}
		if( availSize > 0 && strIns.length() > 0 ) {
			text.insert( viewPos + cursorPos, strIns.substr( 0, sizeToUse ) );
			cursorPos += sizeToUse;
		}
		if( cursorPos < 0 ) {
			if( viewPos > 0 )
				viewPos--;
			cursorPos = 0;
		}
		if( cursorPos > maxSize ) {
			if( viewPos < text.length() )
				viewPos++;
			cursorPos = maxSize;
		}
		
		unsigned int substrLength = text.length() - viewPos;
		
		if( substrLength > maxSize )
			substrLength = maxSize;
		
		boxtext.setString( text.substr( viewPos, substrLength ).c_str() );	
	}
}


void EditBox::setText( std::string_view newText ) {
	this->text = newText;
	setCursorPos( text.size() );
}

//////////////////////////////////////////////////////////////////////
// TextLabel Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TextLabel::TextLabel() :
	str()
{
	type = GE_FONT;
}

TextLabel::~TextLabel() {
	_log->Write( "TextLabel destructor called" );
}

void TextLabel::initialize( std::string &guitextures ) {
	name = "TextLabel";
	GUIElement::initialize( guitextures );
}

void TextLabel::onLeftMouseDown() {
	GUIElement::onLeftMouseDown();	
}

void TextLabel::onLeftMouseRelease() {
	GUIElement::onLeftMouseRelease();
}

void TextLabel::onMouseOver() {

}

void TextLabel::onRender() {

}

void TextLabel::onRightMouseDown() {

}

void TextLabel::onRightMouseRelease() {

}


//////////////////////////////////////////////////////////////////////
// MultiLineTextLabel Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MultiLineTextLabel::MultiLineTextLabel() {
	font.loadFont( "fonts/8x16.bmp" );

	yaddPos = 0;
	yPad = 3;

	name = "MultilineTextLabel";
}

MultiLineTextLabel::~MultiLineTextLabel() {
	
}

void MultiLineTextLabel::onLeftMouseDown() {
	GUIElement::onLeftMouseDown();	
}

void MultiLineTextLabel::onLeftMouseRelease() {
	GUIElement::onLeftMouseRelease();
}

void MultiLineTextLabel::onMouseOver() {

}

void MultiLineTextLabel::onRender() {

}

void MultiLineTextLabel::onRightMouseDown() {

}

void MultiLineTextLabel::onRightMouseRelease() {

}

void MultiLineTextLabel::addLines( int numLines ) {
	for( int i = 0; i < numLines; i++ ) {
		auto label = std::make_unique<TextLabel>( );
		
		label->setX( x );
		label->setY( y + yaddPos );
		
		yaddPos += font.getFontHeight() + yPad;

		label->loadFont( &font );
		addChild( label.get( ) );
		lines.push_back( std::move(label) );

		height = getNumLines() * ( font.getFontHeight() + yPad );	
	}
}

void MultiLineTextLabel::setX( GuiDim val ) {
	x = val;
	auto i = lines.begin();
	for( ; i != lines.end(); i++ ) {
		(*i)->setX( val );
	}
}

void MultiLineTextLabel::setY( GuiDim val ) {
	auto i = lines.begin();
	y = val;
	GuiDim curry = y;
	for( ; i != lines.end(); i++ ) {
		(*i)->setY( curry );
		curry += font.getFontHeight() + yPad;
	}
}

bool MultiLineTextLabel::fillLines( const std::string &in ) {
	//tokenize input string
	CgtLib::StringTokenizer st;
	st.setMinTokens( getNumLines() );
	st.setDelims( "\n" );
	auto tokens = st.tokenize( in );

	//ensure we have enough lines to follow this request
	if( tokens.size() > getNumLines() ) {
		addLines( tokens.size() - getNumLines() );
	}

	//fill lines
	auto i = lines.begin();
	for( int ct = 0; i != lines.end(); i++, ct++ ) {
		(*i)->setString( (tokens)[ ct ] ); 
	}	
	return true;
}

//////////////////////////////////////////////////////////////////////
// Font Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Font::Font( ) {
	fontWidth = 0;
	fontHeight = 0;
	currChar = 0;
}

Font::~Font() {
}

void Font::loadFont( const char *file ) {
	try {
		//_log->Write( "Load font %s", file );
	    image = (loadImage( file ));
		fontWidth = (GuiDim)image.sizeX / 32;
		fontHeight = (GuiDim)image.sizeY / 4;	
		image.createGLTexture( );
	}
	catch( ImageException ) {
		std::string error = "Error loading font: ";
		error += file;
		_log->Write( error.c_str() );
	}

	//prebuild coordinates for all chars of this font
	buildCoordTable();
}

//coordinates for fonts are precalculated ( for speed - avoids multiple divides 
//every frame when console is showing )
TextureCoord *Font::getCoord( unsigned char c )  {
	return &coordTable[c];
}

void Font::buildCoordTable() {
	GuiDim x, y;
	for( ubyte c = 0; c < 255; c++ ) {
		int gridx;
		int gridy = 0;
		int pos = ( int )c;
		
		//calculate gridy
		while( pos >= 32 ) {
			pos -= 32;
			gridy++;		
		}
		
		//calculate gridx
		pos = ( int )c;
		gridx = pos - ( gridy * 32 );

		//calculate x , and y
		x = gridx * fontWidth;
		y = ( 3 * fontHeight ) - ( gridy * fontHeight );	
		
		TextureCoord tc;
		tc.x0 = x / image.sizeX;
		tc.y0 = y / image.sizeY;
		tc.x1 = ( x + fontWidth ) / image.sizeX;
		tc.y1 = ( y + fontHeight ) / image.sizeY;

		coordTable.push_back( tc );
	}
}