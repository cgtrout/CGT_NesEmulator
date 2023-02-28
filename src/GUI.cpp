#include "precompiled.h"

#include "GUI.h"
#include "GLGeneral.h"

using namespace GUISystem;
//using namespace Render;

using namespace FrontEnd;
using namespace InputSystem;
//#include "CgString.h"
#include <algorithm>
#include <functional>

//TODO use "safe" string functions
#if _MSC_VER > 1000
	#pragma warning( disable : 4996 )
#endif
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

GUI::GUI() {
	usingMouse = true;
	//usingKeyboard = false;
	
	//drawDebugLines = false;
	drawGUI = true;


	//ConsoleVariable< float > &opacity = guiOpacity;
}

GUI::~GUI() {
}

void GUI::initialize( ) {
	consoleSystem->variables.addBoolVariable( &drawDebugLines );
	consoleSystem->variables.addFloatVariable( &guiOpacity );
	consoleSystem->variables.addFloatVariable( &fontOpacity );

	input = Input::getInstance( );
}

/*
==============================================
GUI::runFrame()

  updates all elements based on user's actions
==============================================
*/
void GUI::runFrame() {
	systemMain->timeProfiler.startSection( "Gui" );
	
	GUIElement *activeElem = findElementCursorOver();
	
	if( activeElem != NULL && usingMouse ) {
		activeElem->onMouseOver();
		if( input->isMouseLeftDown() ) {
			unactivateAllElements();
			activeElem->onLeftMouseDown();
		}
		if( input->isMouseLeftUp() )
			activeElem->onLeftMouseRelease();
		if( input->isMouseRightDown() )
			activeElem->onRightMouseDown();
		if( input->isMouseRightUp() )
			activeElem->onRightMouseRelease();
	}
	else {
		if( input->isMouseLeftDown() ) 
			unactivateAllElements();
	}
	
	sendActiveToFront();
	//input->setState( Input::NORMAL_MODE );
	updateElementsList( elements );

	systemMain->timeProfiler.stopSection( "Gui" );
}

/*
==============================================
GUI::updateElementsList()

  update all elements ( and its children ) in list 
  elems given as parameter
==============================================
*/
void GUI::updateElementsList(std::vector< GUIElement* > elems ) {
	GUIElement *curr;
	std::vector< GUIElement* >::iterator iter;
	for( iter = elems.begin(); iter != elems.end(); iter++ ) {
		curr = ( GUIElement* )( *iter );
		if( !curr->isOpen() ) {
			continue;
		}
		if( input->getLastKeyPressed() != 0 && curr->isActiveElement() ) {
			curr->onKeyDown( input->getLastKeyPressed() );
		}
		if( curr->getType() == GE_EDITBOX ) {
			if( curr->isActiveElement() && usingMouse ) {
				input->setState( Input::TYPE_MODE );
				curr->update();
			}
			else {
				input->setState( Input::NORMAL_MODE );
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

  returns NULL if no element is found
==============================================
*/
GUIElement *GUI::findElementCursorOver() {
	std::vector< GUIElement* > currlist;
	GUIElement *currelem = NULL;
	GUIElement *retelem = NULL;

	int mousex = input->getMouseX();
	int mousey = input->getMouseY();

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
			currelem = NULL;
		}
		if( currelem == NULL )
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
	std::vector< GUIElement* >::reverse_iterator iter;
	GUIElement *curr;

	for( iter = elems.rbegin(); iter != elems.rend(); iter++ ) {
		curr = ( GUIElement* )( *iter );

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

GUIElement::GUIElement( std::string guitextures ) {
}

GUIElement::~GUIElement() {
	_log->Write( "GUIElement destructor called type=%d", this->getType() );
}

void GUIElement::initialize( std::string guitextures )
 {
	x = 0; y = 0; width = 128; height = 128;
	type = GE_BASE;
	parent = NULL;

	open = true;
	activeElement = false;

	try {
		gt.loadFile( guitextures );
	}
	catch( GUITextures::GUITexturesLoadException ) {
		throw GUIElementInitializeException( "GUIElement::initialize", "Error loading GUI Texture file" );
	}
	parent = NULL;
}

/*
==============================================
GUIElement *GUIElement::getRootParent()
==============================================
*/
GUIElement *GUIElement::getRootParent() {
	GUIElement *ret = this;
	while( ret->parent != NULL ) {
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
	GUIElement *curr = NULL;

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
void GUITextures::loadFromFile( const std::string &fileName ) {
	//char buf[ 1000 ];
	//std::string line;
	std::ifstream is( fileName );
	std::string error;
	this->fileName = fileName;

	if( is.fail() != 0 ) {
		std::stringstream ss;
		ss << "Error loading file \"" << fileName << "\"" << strerror(is.fail());
		throw GUITexturesLoadException( "GUITextures::loadFromFile", ss.str() );
	}

	for ( std::string line; std::getline( is, line ); )
	{
		parseLine( line );
	}
	
	is.close();
}

void GUITextures::parseLine( const std::string &line ) {
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
	fileList.push_back( line.substr( firstq+1, secq-firstq-1 ) );
}

//////////////////////////////////////////////////////////////////////
// DialogBox Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DialogBox::DialogBox() {
	initialize( "gui/dialogbox/dialogbox.gt" );

}

DialogBox::DialogBox( std::string guitextures ) {
	initialize( guitextures );
}

void DialogBox::initialize( std::string guitextures ) {
	GUIElement::initialize( guitextures );
	type = GE_DIALOGBOX;
	
	titleBar = new TitleBar();
	font = new Font();

	font->loadFont( "fonts/8x16.bmp" );
	
	title = new DialogTitle();
	title->setString( "Dialog" );
	title->loadFont( font );
		
	titleBar->setX( x ); 
	titleBar->setY( y ); 
	titleBar->setWidth( width );
	titleBar->setDialogTitle( title );

	try {
		lborder.image = loadImage( gt.getNextTexture() );
		rborder.image = loadImage( gt.getNextTexture() );
		blcorner.image = loadImage( gt.getNextTexture() );
		brcorner.image = loadImage( gt.getNextTexture() );
		bborder.image = loadImage( gt.getNextTexture() );
		background.image = loadImage( gt.getNextTexture() );

		lborder.image = convertToAlpha( 0,0,0,lborder.image );
		rborder.image = convertToAlpha( 0,0,0,rborder.image );
		blcorner.image = convertToAlpha( 0,0,0,blcorner.image );
		brcorner.image = convertToAlpha( 0,0,0,brcorner.image );
		bborder.image = convertToAlpha( 0,0,0,bborder.image );
		background.image = convertToAlpha( 0,0,0,background.image );

		createTexture( lborder.image, &lborder.imageid );
		createTexture( rborder.image, &rborder.imageid );
		createTexture( blcorner.image, &blcorner.imageid );
		createTexture( brcorner.image, &brcorner.imageid );
		createTexture( bborder.image, &bborder.imageid );
		createTexture( background.image, &background.imageid );
	}
	catch( ImageException ) {
		throw GUIElementInitializeException( "DialogBox: ", "error loading image" );
	}
	
	addChild( titleBar );	
}

DialogBox::~DialogBox() {
	//delete lborder.image;
	//delete rborder.image;
	//delete blcorner.image;
	//delete brcorner.image;
	//delete bborder.image;
	delete titleBar;
	delete font;
	delete title;
}

void DialogBox::onLeftMouseDown() {
	GUIElement::onLeftMouseDown();	
}

void DialogBox::onLeftMouseRelease() {
	GUIElement::onLeftMouseRelease();	
}

void DialogBox::onMouseOver() {

}

void DialogBox::onRender() {
	drawList.clear();

	lborder.x = x;
	lborder.y = y + titleBar->left.image.sizeY;
	lborder.stretchFactory = height - ( blcorner.image.sizeY+1 ) - titleBar->left.image.sizeY;
	lborder.stretchType = ST_Y;

	rborder.x = x + ( width - rborder.image.sizeX );
	rborder.y = y + titleBar->right.image.sizeY;
	rborder.stretchFactory = height - ( brcorner.image.sizeY+1 ) - titleBar->right.image.sizeY;
	rborder.stretchType = ST_Y;

	blcorner.x = x;
	blcorner.y = y + ( height - blcorner.image.sizeY );
	blcorner.stretchType = ST_NONE;

	brcorner.x = x + ( width - brcorner.image.sizeX );
	brcorner.y = y + ( height - blcorner.image.sizeY );
	brcorner.stretchType = ST_NONE;

	bborder.x = x + ( blcorner.image.sizeX );
	bborder.y = y + ( height - blcorner.image.sizeY );
	bborder.stretchFactorx = width - ( 2 * brcorner.image.sizeX );
	bborder.stretchType = ST_X;

	background.x = x + lborder.image.sizeX;
	background.y = y + titleBar->left.image.sizeY;
	background.stretchType = ST_XY;
	background.stretchFactorx = width - rborder.image.sizeX;
	background.stretchFactory = height - ( titleBar->middle.image.sizeY + bborder.image.sizeY );

	drawList.push_back( &lborder );
	drawList.push_back( &rborder );
	drawList.push_back( &brcorner );
	drawList.push_back( &blcorner );
	drawList.push_back( &bborder );
	drawList.push_back( &background );
}

void DialogBox::onRightMouseDown() {

}

void DialogBox::onRightMouseRelease() {

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
		l.image = loadImage( gt.getNextTexture() );
		r.image = loadImage( gt.getNextTexture() );
		b.image = loadImage( gt.getNextTexture() );
		t.image = loadImage( gt.getNextTexture() );
		bl.image = loadImage( gt.getNextTexture() );
		br.image = loadImage( gt.getNextTexture() );
		tl.image = loadImage( gt.getNextTexture() );
		tr.image = loadImage( gt.getNextTexture() );
		cursor.image = loadImage( gt.getNextTexture() );
		
		//TODO what if a tga file with alpha is specified??
		l.image = convertToAlpha( 255,255,255,l.image );
		r.image = convertToAlpha( 255,255,255,r.image );
		b.image = convertToAlpha( 255,255,255,b.image );
		t.image = convertToAlpha( 255,255,255,t.image );
		bl.image = convertToAlpha( 255,255,255,bl.image );
		br.image = convertToAlpha( 255,255,255,br.image );
		tl.image = convertToAlpha( 255,255,255,tl.image );
		tr.image = convertToAlpha( 255,255,255,tr.image );
		cursor.image = convertToAlpha( 0,0,0,cursor.image );

		createTexture( l.image, &l.imageid );
		createTexture( r.image, &r.imageid );
		createTexture( b.image, &b.imageid );
		createTexture( t.image, &t.imageid );
		createTexture( bl.image, &bl.imageid );
		createTexture( br.image, &br.imageid );
		createTexture( tl.image, &tl.imageid );
		createTexture( tr.image, &tr.imageid );
		createTexture( cursor.image, &cursor.imageid );
	}
	catch( ImageException ) {
		throw GUIElementInitializeException( "EditBox", "loading image" );
	}
}

EditBox::~EditBox() {
}

void EditBox::onLeftMouseDown() {
	input->setUseDelay( true );
	blinkTimeLast = timer->getCurrTime();
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

		drawList.push_back( &l );
		drawList.push_back( &r );
		drawList.push_back( &bl );
		drawList.push_back( &br );
		drawList.push_back( &b );
		drawList.push_back( &tl );
		drawList.push_back( &t );
		drawList.push_back( &tr );
	}

	//control blinking of position cursor
	if( activeElement ) {
		elapsedTime += timer->getCurrTime();

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

		if( input->isKeyDown( KB_RETURN ) ) {
			onEnterKey();
		}

		if( input->isKeyDown( KB_HOME ) ) {
			cursorPos = 0;
			viewPos = 0;
		}
		if( input->isKeyDown( KB_END ) ) {
			if( text.length() < maxSize ) {
				cursorPos = text.length();
				viewPos = 0;
			}
			else {
				cursorPos = maxSize;
				viewPos = text.length() - maxSize;
			}			                             
		}
		if( input->isKeyDown( KB_LEFT ) ) {
			if( viewPos + cursorPos > 0 )
				--cursorPos;
		}
		if( input->isKeyDown( KB_RIGHT ) ) {
			if( viewPos + cursorPos < text.length() )
				++cursorPos;
		}
		if( input->isKeyDown( KB_BACK ) ) {
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
		if( input->isKeyDown( KB_DELETE ) ) {
			if( text.length() > 0 ) {
				text.erase( viewPos + cursorPos, 1 );
			}
		}
		std::string strIns;
		int x = 0;
		while( input->isMoreKeyMessages() ) {
			strIns += input->getNextKeyMessage();
		}
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

void EditBox::setText( const char *text ) {
	if( text != NULL ) {
		this->text = text;
	}
	setCursorPos( this->text.size() );
}

void EditBox::setText( std::string text ) {
	this->text = text;
	setCursorPos( text.size() );
}


/*
=================================================================
=================================================================
Button Class
=================================================================
=================================================================
*/
Button::Button() {
	initialize( "gui/button/button.gt" );
}

Button::Button( std::string guitextures ) {
	initialize( guitextures );
}

Button::~Button() {
}

void Button::initialize( std::string guitextures ) {
	GUIElement::initialize( guitextures );
	buttonDown = false;
	
	try {
		l.image = loadImage( gt.getNextTexture() );
		r.image = loadImage( gt.getNextTexture() );
		b.image = loadImage( gt.getNextTexture() );
		t.image = loadImage( gt.getNextTexture() );
		bl.image = loadImage( gt.getNextTexture() );
		br.image = loadImage( gt.getNextTexture() );
		tl.image = loadImage( gt.getNextTexture() );
		tr.image = loadImage( gt.getNextTexture() );
		
		dl.image = loadImage( gt.getNextTexture() );
		dr.image = loadImage( gt.getNextTexture() );
		db.image = loadImage( gt.getNextTexture() );
		dt.image = loadImage( gt.getNextTexture() );
		dbl.image = loadImage( gt.getNextTexture() );
		dbr.image = loadImage( gt.getNextTexture() );
		dtl.image = loadImage( gt.getNextTexture() );
		dtr.image = loadImage( gt.getNextTexture() );

		l.image = convertToAlpha( 0,0,0,l.image );
		r.image = convertToAlpha( 0,0,0,r.image );
		b.image = convertToAlpha( 0,0,0,b.image );
		t.image = convertToAlpha( 0,0,0,t.image );
		bl.image = convertToAlpha( 0,0,0,bl.image );
		br.image = convertToAlpha( 0,0,0,br.image );
		tl.image = convertToAlpha( 0,0,0,tl.image );
		tr.image = convertToAlpha( 0,0,0,tr.image );

		dl.image = convertToAlpha( 0,0,0,dl.image );
		dr.image = convertToAlpha( 0,0,0,dr.image );
		db.image = convertToAlpha( 0,0,0,db.image );
		dt.image = convertToAlpha( 0,0,0,dt.image );
		dbl.image = convertToAlpha( 0,0,0,dbl.image );
		dbr.image = convertToAlpha( 0,0,0,dbr.image );
		dtl.image = convertToAlpha( 0,0,0,dtl.image );
		dtr.image = convertToAlpha( 0,0,0,dtr.image );

		createTexture( l.image, &l.imageid );
		createTexture( r.image, &r.imageid );
		createTexture( b.image, &b.imageid );
		createTexture( t.image, &t.imageid );
		createTexture( bl.image, &bl.imageid );
		createTexture( br.image, &br.imageid );
		createTexture( tl.image, &tl.imageid );
		createTexture( tr.image, &tr.imageid );

		createTexture( dl.image, &dl.imageid );
		createTexture( dr.image, &dr.imageid );
		createTexture( db.image, &db.imageid );
		createTexture( dt.image, &dt.imageid );
		createTexture( dbl.image, &dbl.imageid );
		createTexture( dbr.image, &dbr.imageid );
		createTexture( dtl.image, &dtl.imageid );
		createTexture( dtr.image, &dtr.imageid );
	}
	catch( ImageException ) {
		throw GUIElementInitializeException( "Button: error", "loading image" );
	}
}

void Button::onLeftMouseDown() {
	GUIElement::onLeftMouseDown();	
	buttonDown = true;
}

void Button::onLeftMouseRelease() {
	GUIElement::onLeftMouseRelease();

	buttonDown = false;
	onButtonPress();
}

void Button::onMouseOver() {

}

void Button::onRightMouseDown() {
}

void Button::onRightMouseRelease() {
}

void Button::onRender() {
	drawList.clear();

	if( !buttonDown ) {
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
	
		drawList.push_back( &l );
		drawList.push_back( &r );
		drawList.push_back( &bl );
		drawList.push_back( &br );
		drawList.push_back( &b );
		drawList.push_back( &tl );
		drawList.push_back( &t );
		drawList.push_back( &tr );
	}
	else {
		dl.x = x;
		dl.y = y + t.image.sizeY;
		dl.stretchFactory = height - ( 2 * b.image.sizeY );
		dl.stretchType = ST_Y;

		dr.x = x + ( width - ( r.image.sizeX ) );
		dr.y = y + t.image.sizeY;
		dr.stretchFactory = height - ( 2 * b.image.sizeY );
		dr.stretchType = ST_Y;

		dbl.x = x;
		dbl.y = y + ( height - bl.image.sizeY );
		dbl.stretchType = ST_NONE;

		dbr.x = x + ( width - br.image.sizeX );
		dbr.y = y + ( height - bl.image.sizeY );
		dbr.stretchType = ST_NONE;

		db.x = x + ( bl.image.sizeX );
		db.y = y + ( height - bl.image.sizeY );
		db.stretchFactorx = width - ( 2 * br.image.sizeX );
		db.stretchType = ST_X;
		
		dtl.x = x;
		dtl.y = y;
		dtl.stretchType = ST_NONE;
		
		dt.x = x + tl.image.sizeX;
		dt.y = y;
		dt.stretchFactorx = width - ( 2 * tr.image.sizeX );
		dt.stretchType = ST_X;

		dtr.x = x + ( width - tr.image.sizeX );
		dtr.y = y;
		dtr.stretchType = ST_NONE;

		drawList.push_back( &dl );
		drawList.push_back( &dr );
		drawList.push_back( &dbl );
		drawList.push_back( &dbr );
		drawList.push_back( &db );
		drawList.push_back( &dtl );
		drawList.push_back( &dt );
		drawList.push_back( &dtr );
	}
}

void Button::update() {
	if( buttonDown ) {
		if( input->isMouseLeftUp() )
			buttonDown = false;
	}
}

void Button::onButtonPress() {

}

void CloseButton::onButtonPress() {
	getRootParent()->close();
}

//////////////////////////////////////////////////////////////////////
// TitleBar Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TitleBar::TitleBar() {
	initialize( "gui/titlebar/titlebar.gt" );
}

TitleBar::TitleBar( std::string guitextures ) {
	initialize( guitextures );
}

TitleBar::~TitleBar() {
	delete closeButton;
}

void TitleBar::initialize( std::string guitextures ) {
	GUIElement::initialize( guitextures );
	movement = false;

	closeButton = new CloseButton();
	closeButton->setX( x + width - 5 );
	closeButton->setY( y + ( height / 2 ) );
	closeButton->setWidth( 16 );
	closeButton->setHeight( 16 );
	
	try {
		left.image = loadImage( gt.getNextTexture() );
		right.image = loadImage( gt.getNextTexture() );
		middle.image = loadImage( gt.getNextTexture() );

		left.image = convertToAlpha( 0,0,0,left.image );
		right.image = convertToAlpha( 0,0,0,right.image ); 
		middle.image = convertToAlpha( 0,0,0,middle.image );

		createTexture( left.image, &left.imageid );
		createTexture( right.image, &right.imageid );
		createTexture( middle.image, &middle.imageid );
	}
	catch( ImageException ) {
		delete closeButton;
		throw GUIElementInitializeException( "TitleBar: error", "loading image" );
	}

	height = (GuiDim)middle.image.sizeY;

	addChild( closeButton );
}

void TitleBar::onLeftMouseDown() {
	lastx = input->getMouseX();
	lasty = input->getMouseY();	
	movement = true;

	parent->onLeftMouseDown();
}

void TitleBar::onLeftMouseRelease() {
	movement = false;
	parent->onLeftMouseRelease();
}

void TitleBar::onMouseOver() {

}

void TitleBar::onRender() {
	drawList.clear();

	x = parent->getX();
	y = parent->getY();
	width = parent->getWidth();
	height = (GuiDim)middle.image.sizeY;

	left.x = parent->getX();
	left.y = parent->getY();
	left.stretchType = ST_NONE;
	
	middle.x = parent->getX() + left.image.sizeX;
	middle.y = parent->getY();
	middle.stretchFactorx = parent->getWidth() - ( 2 * right.image.sizeX );
	middle.stretchType = ST_X;

	right.x = parent->getX() + ( parent->getWidth() - right.image.sizeX );
	right.y = parent->getY();
	right.stretchType = ST_NONE;

	drawList.push_back( &left );
	drawList.push_back( &right );
	drawList.push_back( &middle );

	textLabel->setX( parent->getX()+4 );
	textLabel->setY( parent->getY()+4 );
	closeButton->setX( getRootParent()->getX() + getRootParent()->getWidth()-20 );
	closeButton->setY( getRootParent()->getY() + 5 );
}

void TitleBar::onRightMouseDown() {

}

void TitleBar::onRightMouseRelease() {

}

void TitleBar::update() {
	if( movement ) {
		( getRootParent() )->move( input->getMouseX() - lastx, input->getMouseY() - lasty );
		
		lastx = input->getMouseX();
		lasty = input->getMouseY();	
	}
}

//////////////////////////////////////////////////////////////////////
// SliderBar Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Slider::SliderBar::SliderBar( int type ) {
	this->type = type;
	if( type == SLIDER_Y )
		initialize( "gui/SliderY/sliderbary.gt" );
	else
		initialize( "gui/SliderX/sliderbarx.gt" );
}

Slider::SliderBar::SliderBar( std::string guitextures, int type ) {
	this->type = type;
	initialize( guitextures );	
}

Slider::SliderBar::~SliderBar() {
}

void Slider::SliderBar::initialize( std::string guitextures ) {
	GUIElement::initialize( guitextures );
	try {
		if( type == SLIDER_Y ) {
			sliderBar.image = loadImage( gt.getNextTexture() );
		}	
		else if( type == SLIDER_X ) {
			sliderBar.image = loadImage( gt.getNextTexture() );
		}
		sliderBar.image = convertToAlpha( 255,255,255, sliderBar.image );
		createTexture( sliderBar.image, &sliderBar.imageid );		
	}
	catch( ImageException ) {
		throw GUIElementInitializeException( "SliderBar:", "error loading image" );
	}
	
	width = 20;
	height = 10;
}

void Slider::SliderBar::onLeftMouseDown() {
	parent->onLeftMouseDown();	
}

void Slider::SliderBar::onLeftMouseRelease() {
	parent->onLeftMouseRelease();
}

void Slider::SliderBar::onMouseOver() {
}

void Slider::SliderBar::onRender() {
	if( type == SLIDER_Y ) {
		x = getParent()->getX();
		y = getParent()->getY()+( ( Slider* )parent )->calcSliderPos();

		drawList.clear();
		
		sliderBar.stretchType = ST_NONE;
		sliderBar.x = parent->getX() + ( parent->getWidth()/2 ) - 10;	//x - ( center point of SliderBar )
		sliderBar.y = y;
	}
	else if( type == SLIDER_X ) {
		x = getParent()->getX()+( ( Slider* )parent )->calcSliderPos();
		y = getParent()->getY() ;

		drawList.clear();
		
		sliderBar.stretchType = ST_NONE;
		sliderBar.x = x;
		sliderBar.y = parent->getY() - ( parent->getHeight()/2 ) + 10;
	}
	
	drawList.push_back( &sliderBar );
}

void Slider::SliderBar::onRightMouseDown() {
}

void Slider::SliderBar::onRightMouseRelease() {
}

void Slider::SliderBar::update() {
}


//////////////////////////////////////////////////////////////////////
// Slider Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
Slider::Slider( int type ) {
	this->type = type;
	if( type == SLIDER_Y )
		initialize( "gui/SliderY/slidery.gt" );
	else
		initialize( "gui/SliderX/sliderx.gt" );
}

Slider::Slider( std::string guitextures, int type ) {
	this->type = type;
	initialize( guitextures );
}

Slider::~Slider() {
	delete sliderBar;
}

void Slider::initialize( std::string guitextures ) {
	GUIElement::initialize( guitextures );
	drawValueLabel = false;
	this->type = type;
	if( type != SLIDER_X && type != SLIDER_Y )
		throw GUIElementInitializeException( "Slider:", "Invalid slider type" );
	value = 0;
	
	if( type == SLIDER_Y )
		width = 20;	
	else if( type == SLIDER_X )
		height = 20;

	velocity = 16000;
	slideControl = false;

	sliderBar = new SliderBar( type );
	font.loadFont( "fonts/8x16.bmp" );
	valueLabel.loadFont( &font );
	valueLabel.setOpen( false );
			
	try {
		if( type == SLIDER_Y ) {
			top.image = loadImage( gt.getNextTexture() );
			bottom.image = loadImage( gt.getNextTexture() );
			middle.image = loadImage( gt.getNextTexture() );
		}
		else if( type == SLIDER_X ) {
			top.image = loadImage( gt.getNextTexture() );
			bottom.image = loadImage( gt.getNextTexture() );
			middle.image = loadImage( gt.getNextTexture() );
		}

		top.image = convertToAlpha( 0,0,0, top.image );
		bottom.image = convertToAlpha( 0,0,0, bottom.image );
		middle.image = convertToAlpha( 0,0,0, middle.image );
		
		createTexture( top.image, &top.imageid );
		createTexture( bottom.image, &bottom.imageid );
		createTexture( middle.image, &middle.imageid );
	}
	catch( ImageException ) {
		throw GUIElementInitializeException( "Slider:", "Error loading texture" );
	}
	
	addChild( sliderBar );
	addChild( &valueLabel );
}

void Slider::onLeftMouseDown() {
	if( !slideControl ) {
		//store mouse vals
		mouseX = input->getMouseX();
		mouseY = input->getMouseY();
	}
	
	slideControl = true;

	getParent()->onLeftMouseDown();
}

void Slider::onLeftMouseRelease() {
	if( slideControl ) {
		slideControl = false;
	}
}

void Slider::onMouseOver() {
}

void Slider::onRender() {
	drawList.clear();
	
	if( type == SLIDER_Y ) {
		top.stretchType = ST_NONE;
		top.x = x + ( width/2 );
		top.y = y;

		middle.stretchType = ST_Y;
		middle.x = x + ( width/2 );
		middle.y = y + top.image.sizeY;
		middle.stretchFactory = height - top.image.sizeY - bottom.image.sizeY;
		
		bottom.stretchType = ST_NONE;
		bottom.x = x + ( width/2 );
		bottom.y = y + height - bottom.image.sizeY;
	}
	else if( type == SLIDER_X ) {
		top.stretchType = ST_NONE;
		top.x = x + width - top.image.sizeX;
		top.y = y + ( height/2 );

		middle.stretchType = ST_X;
		middle.x = x + bottom.image.sizeX;
		middle.y = y + ( height/2 );
		middle.stretchFactorx = width - top.image.sizeY - bottom.image.sizeY;
		
		bottom.stretchType = ST_NONE;
		bottom.x = x;
		bottom.y = y + ( height/2 );
	}

	if( drawValueLabel ) {
		valueLabel.setX( x + 20 );
		valueLabel.setY( y + 20 );
		static char numbuffer[ 30 ];
		static std::string strvalue;
		sprintf( numbuffer, "%x", value );
		strvalue = numbuffer;
		valueLabel.setString( strvalue );
	}

	drawList.push_back( &top );
	drawList.push_back( &middle );
	drawList.push_back( &bottom );
}

void Slider::onRightMouseDown() {

}

void Slider::onRightMouseRelease() {

}

void Slider::update() {
	if( slideControl ) {
		//move slider
		static int delta;
		if( type == SLIDER_Y )
			delta = mouseY - input->getMouseY();
		else if( type == SLIDER_X )
			//delta = mouseX - input->getMouseX();
			delta = input->getMouseX() - mouseX;
		value += delta * velocity;
		
        	//enforce constraints
		//TODO change value constraints to ensure proper functionality
		if( value < 4294967295l && value > 0xAAAAAA ) {
			value = 0;
		}
		else if( value > maxSliderHeight ) {
			value = maxSliderHeight;
		}
	
		//reset mouse pos
		input->setMouseY( mouseY );
		input->setMouseX( mouseX );
		input->moveMouseToNewCoordinates();
	}
}

GuiDim Slider::calcSliderPos() {
	if( type == SLIDER_Y )
		return ( height - 10 ) - ( ( value * ( height - 10 ) ) / ( maxSliderHeight ) );
	else if( type == SLIDER_X )
		return ( value * ( width - 10 ) ) / maxSliderHeight;
	else	
		return 0;
}


/*
=================================================================
=================================================================
DialogTitle Class
=================================================================
=================================================================
*/
void DialogTitle::onLeftMouseRelease() {
	TextLabel::onLeftMouseRelease();
	parent->onLeftMouseRelease();	
}

void DialogTitle::onLeftMouseDown() {
	parent->onLeftMouseDown();
}


//////////////////////////////////////////////////////////////////////
// TextLabel Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TextLabel::TextLabel() {
	type = GE_FONT;
	//font = new Font();
}

TextLabel::~TextLabel() {
	_log->Write( "TextLabel destructor called" );
}

void TextLabel::initialize( std::string &guitextures ) {
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
		TextLabel *l = new TextLabel();
		
		l->setX( x );
		l->setY( y + yaddPos );
		
		yaddPos += font.getFontHeight() + yPad;

		l->loadFont( &font );
		lines.push_back( l );
		addChild( l );

		height = getNumLines() * ( font.getFontHeight() + yPad );	
	}
}

void MultiLineTextLabel::setX( GuiDim val ) {
	x = val;
	std::vector< TextLabel* >::iterator i = lines.begin();
	for( ; i != lines.end(); i++ ) {
		(*i)->setX( val );
	}
}

void MultiLineTextLabel::setY( GuiDim val ) {
	std::vector< TextLabel* >::iterator i = lines.begin();
	y = val;
	GuiDim curry = y;
	for( ; i != lines.end(); i++ ) {
		(*i)->setY( curry );
		curry += font.getFontHeight() + yPad;
	}	
}

bool MultiLineTextLabel::fillLines( const std::string &in ) {
	//tokenize input string
	CgtString::StringTokenizer st;
	st.setMinTokens( getNumLines() );
	st.setDelims( "\n" );
	auto tokens = st.tokenize( in );

	//ensure we have enough lines to follow this request
	if( tokens.size() > getNumLines() ) {
		addLines( tokens.size() - getNumLines() );
	}

	//fill lines
	std::vector< TextLabel* >::iterator i = lines.begin();
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

Font::Font() {
	fontWidth = 0;
	fontHeight = 0;
}

Font::~Font() {
	_log->Write( "Font box destructor called" );
}

void Font::loadFont( const char *file ) {
	try {
		//_log->Write( "Load font %s", file );
	        image = loadImage( file );
		fontWidth = (GuiDim)image.sizeX / 32;
		fontHeight = (GuiDim)image.sizeY / 4;	
		if( image.channels == 3 )
			image = convertToAlpha( 0, 0, 0, image );	
		createTexture( image, &imageid );
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