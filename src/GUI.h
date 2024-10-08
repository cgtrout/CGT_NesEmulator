// GUI.h: interfaces for the GUI classes.
// ISSUES:  Lack of memory management
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <string>
#include <memory>
#include "Image.h"
#include "Input.h"

#include <SDL_keyboard.h>

//to fix name collision
#undef DialogBox

#include "CGTSingleton.h"
using namespace CGTSingleton;

#include "Console.h"

namespace Render {
	class Renderer;
}

namespace GUISystem {
	typedef float GuiDim;
	/*
	=================================================================
	=================================================================
	GEDrawElement

	  a draw element is used to tell the gui how to draw a particular
	  image

	  each GUIElement will have a list of these
	=================================================================
	=================================================================
	*/
	//stretch types
	enum StretchType {
		ST_X,		//stretch image in the x direction
		ST_Y,		//stretch image in the y direction
		ST_XY,		//stretch in x and y direction
		ST_NONE		//draw image unaltered
	};

	class GEDrawElement {
	public:
		GEDrawElement( ) :
			image( ),
			x( 0 ),
			y( 0 ),
			stretchFactorx( 1.0f ),
			stretchFactory( 1.0f ),
			stretchType( ST_XY ),
			opacity( 1.0f ) {
		}

		virtual ~GEDrawElement( ) {
		}

		// Copy constructor
		GEDrawElement( const GEDrawElement& de ) = delete;

		Image image;
		GuiDim x, y,
			stretchFactorx, // How much to stretch if not ST_NONE
			stretchFactory;
		StretchType stretchType;

		float opacity; // Transparency of draw
	};

	/*
	=================================================================
	=================================================================
	GUITextures Class

	simple class used to get filename strings linearly from a file
	=================================================================
	=================================================================
	*/
	class GUITextures {
	public:
		GUITextures( std::string fileName )
			: fileName( fileName ), currLine( 0 ) {}

		GUITextures( )
			: fileName( "" ), currLine( 0 ) {}

		virtual ~GUITextures( ) {}

		void loadFile( std::string_view fileNameIn ) {
			loadFromFile( fileNameIn );
			iter = fileList.begin( );

			if( fileList.size( ) == 0 ) {
				throw GUITexturesLoadException( "GUITextures::loadFile", "No textures in list" );
			}
		}

		// TODO validation to make sure there is another texture to output
		std::string getNextTexture( ) {
			if( iter != fileList.end( ) ) {
				return *iter++;
			}
			else {
				throw GUITexturesLoadException( "GUITextures::getNextTexture", "No more textures in list" );
			}
		}

		// Initialize exception class
		class GUITexturesLoadException : public CgtException {
		public:
			GUITexturesLoadException( std::string header, std::string m, bool s = true )
				: CgtException( header, m, s ) {
			}
		};

	private:
		std::vector<std::string> fileList;
		std::vector<std::string>::iterator iter;
		std::string fileName;
		void loadFromFile( std::string_view fileName );
		void parseLine( std::string_view line );

		int currLine;
	};


	/*
	=================================================================
	=================================================================
	GUIElement   Class

	  base GUIElement class - all GuiElements are extended from this
	  class
	=================================================================
	=================================================================
	*/
	//gui elements type
	enum {
		GE_BASE,
		GE_FONT,
		GE_DIALOGBOX,
		GE_TITLEBAR,
		GE_EDITBOX
	};

	class GUIElement {
	public:
		GUIElement() : name("unnamed"), inputSystem(nullptr) {}
		GUIElement( std::string guitextures );

		virtual ~GUIElement();

		void setInputSystem( FrontEnd::InputSystem::Input* i ) { 
			inputSystem = i; }

		//these "events" are called when certain mouse actions are performed by the user
		virtual void onLeftMouseDown() {setAsActiveElement();}
		virtual void onLeftMouseRelease() {}
		virtual void onMouseOver() {}
		virtual void onRightMouseDown() {}
		virtual void onRightMouseRelease() {}

		//called when user presses a key on keyboard while a control is selected
		virtual void onKeyDown( SDL_Keycode ) {}
		
		//called when gui is rendering the guielements
		virtual void onRender() {}

		//updates this gui object for next frame
		virtual void update() {}

		//move this gui element and all of its children
		void move( int xm, int ym );
		
		//getters and setters
		GuiDim getX() { return x;}
		GuiDim getY() { return y;}
		GuiDim getWidth() { return width;}
		GuiDim getHeight() { return height;}
		void setX( GuiDim val ) { x = val;}
		void setY( GuiDim val ) { y  = val;}

		void setWidth( GuiDim val ) {width = val;}
		void setHeight( GuiDim val ) {height = val;}

		int getType() {return type;}
		
		void setParent( GUIElement *par ) {parent = par;}
		GUIElement *getParent() {return parent;}

		//add child to this elements childList
		void addChild( GUIElement *elem ) {
			elem->setParent( this );
			elem->setInputSystem( inputSystem );
			children.push_back( elem );
		}
		
		//which child is the active child
		GUIElement *getActiveChild() {
			return getActiveChild( children );
		}
		
		//is this element currently the active element?
		inline bool isActiveElement() {return activeElement;}
		
		//set this element as the active elemeent
		inline void setAsActiveElement() {
			activeElement = true;
			getRootParent()->activeElement = true;
		}
		
		//set this element not to be the active element
		inline void unactivateElement() {
			activeElement = false;
			unactivateChildren( children );
		}
		
		//if a gui element is open it is displayed on the screen; likewise when
		//it is not open it will not be displayed on the screen
		inline bool isOpen() {return open;}
		inline void setOpen( bool val ) {open = val;}
		void close() {open = false;}

		GUIElement *getRootParent();
		
		//initialize exception class
		class GUIElementInitializeException : public CgtException {
		public:
			GUIElementInitializeException( std::string_view header, std::string_view message, bool show = true ) :
				CgtException( header, message, show ) 
			{}
		};

		friend class GUI;

	protected:

		FrontEnd::InputSystem::Input* inputSystem;

		std::string name;
		//every gui element will have 0 or more children.  Children are guielements
		//contained within this gui element
		std::vector< GUIElement* > children;

		//list of draw elements
		std::vector< GEDrawElement* >drawList;

		//gui textures object
		GUITextures gt;

		GuiDim x		= 0;
		GuiDim y		= 0;	
		GuiDim width	= 0;
		GuiDim height	= 0;
		int type		= 0;
		
		bool activeElement = false;;
		bool open = true;

		//if this element is a child to another gui element than this will point to
		//the parent gui element
		GUIElement *parent = nullptr;

		virtual void initialize( std::string guitextures );	
		
		void unactivateChildren( std::vector< GUIElement* > childList );
		GUIElement *getActiveChild( std::vector< GUIElement* > childList );
	};
	/*
	=================================================================
	=================================================================
	Font   Class
	=================================================================
	=================================================================
	*/
	struct TextureCoord {
			GuiDim x0, y0, x1, y1;	

			TextureCoord( ): x0(0), y0(0), x1(0), y1(0) {}
	};

	class Font {
	public:
		Font();
		~Font();

		void loadFont( const char *file );

		GuiDim &getFontWidth() {return fontWidth;}
		GuiDim &getFontHeight() {return fontHeight;}

		Image& getImage() {return image;}

		//pixel coordinates to draw given character at
		//must call setChar first to get these
		TextureCoord *getCoord( unsigned char c );

	private:
		char currChar;
		GuiDim fontWidth;
		GuiDim fontHeight;
	
		//builds the texture coordinate lookup table
		void buildCoordTable();

		//current coordinates
		TextureCoord currCoord;

		std::vector< TextureCoord > coordTable;

		Image image;
	};

	/*
	=================================================================
	=================================================================
	TextLabel Class
	=================================================================
	=================================================================
	*/
	class TextLabel : public GUIElement {
	public:
		TextLabel();
		TextLabel( std::string guitextures );
		~TextLabel();

		void onLeftMouseDown();
		void onLeftMouseRelease();
		void onMouseOver();
		void onRender();
		void onRightMouseDown();
		void onRightMouseRelease();

		void onKeyDown( SDL_Keycode ) {}

		void update() {}

		void loadFont( Font* fontIn ) {this->font = fontIn;}
		
		std::string &getString() {return str;}
		
		void setString ( std::string &n ) { setString( n.c_str() ); }
		
		void setString( const char *n ) {
			str = n;
			height = font->getFontHeight();
			width = font->getFontWidth() * str.length();
		}
		
		Font *getFont() {return font;}

	protected:
		Font *font;
		std::string str;
		void initialize( std::string &guitextures );
	};

	class MultiLineTextLabel : public GUIElement {
	public:
		MultiLineTextLabel();
		MultiLineTextLabel( std::string guitextures );
		~MultiLineTextLabel();

		void onLeftMouseDown();
		void onLeftMouseRelease();
		void onMouseOver();
		void onRender();
		void onRightMouseDown();
		void onRightMouseRelease();

		void onKeyDown( SDL_Keycode ) {}

		void update() {}

		unsigned int getNumLines() { return lines.size(); }

		void addLines( int numLines );

		void setX( GuiDim val );
		void setY( GuiDim val );

		//fills text labels with multi-lined string "in"
		bool fillLines( const std::string &in );
	private:
		//store as unique pointer since we want to pass these pointers around and
		//automatically deallocate with this class
		std::vector< std::unique_ptr<TextLabel> > lines;
		Font font;

		GuiDim yaddPos;	//current y position (of next 
						//line to be added )
		GuiDim yPad;	//amount of y padding
	};

	class DialogTitle : public TextLabel {
		void onLeftMouseRelease();
		void onLeftMouseDown();
	};

	/*
	=================================================================
	=================================================================
	EditBox Class
	=================================================================
	=================================================================
	*/
	class EditBox : public GUIElement {
	public:	
		EditBox();
		EditBox( std::string guitextures );
		~EditBox();
		
		void onLeftMouseDown();
		void onLeftMouseRelease();
		void onMouseOver();
		void onRender();
		void onRightMouseDown();
		void onRightMouseRelease();
		virtual void onEnterKey();

		virtual void onKeyDown( SDL_Keycode ) {}

		void update();

		std::string getText() {return text;}
		void setText( std::string_view text );
		void clearText() {
			text = "";
			boxtext.setString( "" );
			cursorPos = 0;
			viewPos = 0;
		}

		void setCursorPos( int pos ) {cursorPos = pos;}
		void setMaxLength( int len ) { maxLength = len; }
	protected:
		GEDrawElement l, r, b, t, bl, br, tl, tr, cursor;	//left , right, bottom, etc..
		bool multiLine;
		bool draw;

		float blinkTimeWait;
		float blinkTimeLast;
		float elapsedTime;

		unsigned int cursorPos;
		unsigned int boxcursorPos;
		unsigned int viewPos;
		bool showCursor;
		unsigned int maxLength;

		std::string text;
		
		class EditBoxText : public TextLabel {
		public:
			void onLeftMouseDown() {parent->onLeftMouseDown();}
		}boxtext;
		
		Font font;

		void initialize( std::string guitextures );
	};

	/*
	=================================================================
	=================================================================
	GUI   Class
	=================================================================
	=================================================================
	*/
	class GUI {
	public:
		GUI( FrontEnd::InputSystem::Input *inputSystem  );
		virtual ~GUI( );

		//updates all elements based on user's actions
		void runFrame();
		
		//renders ui
		void render();
		
		//adds a gui element to the gui system - caller is responsible for managing lifetime of object
		void addElement( GUIElement* ge );

		//sets whether the gui is using the mouse
		void setUsingMouse( bool val ) {usingMouse = val;}
		
		//is the gui currently using the mouse?
		bool isUsingMouse() {return usingMouse;}

		//unactivates all elements in the elements list
		void unactivateAllElements();

		//initialize exception class
		class GUIRunException : public CgtException {
		public:
			GUIRunException( std::string header, std::string m, bool s = true ) :
				CgtException( header, m, s )
			{}
		};

		void initialize( );

	private:

		Render::Renderer* renderSystem;
		FrontEnd::InputSystem::Input* inputSystem;

		std::vector< GUIElement* > elements;
		
		bool usingMouse;
		//bool usingKeyboard;

		//used so elements don't overlap when drawn overtop of eachother
		float zdrawpos;

		void updateElementsList( std::vector< GUIElement* > elems );
		void renderElements( std::vector< GUIElement* > elems );
		void renderTextLabel( TextLabel *fs );
		void renderDebugLines( GUIElement *element );
		void renderDrawList( std::vector< GEDrawElement* > &drawList );
		GUIElement *findElementCursorOver();
		void sendActiveToFront();

		//bool drawDebugLines;
		bool drawGUI;
	};

}	//namespace gui
