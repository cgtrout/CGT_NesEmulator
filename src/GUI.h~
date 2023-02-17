// GUI.h: interfaces for the GUI classes.
// ISSUES:  Lack of memory management
//
//////////////////////////////////////////////////////////////////////

#if !defined( AFX_GUI_H__B172BAD5_6BBC_442E_B396_1207A4FAFF65__INCLUDED_ )
#define AFX_GUI_H__B172BAD5_6BBC_442E_B396_1207A4FAFF65__INCLUDED_

#include < list >
#include "Image.h"

//to fix name collision
#undef DialogBox

#include "CGTSingleton.h"
using namespace CGTSingleton;

#include "Console.h"

//extern FrontEnd::InputSystem::Input *input;

//TODO why so many instances of classes instantiated with new?
namespace GUISystem {
	typedef float GuiDim ;
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
		GEDrawElement() {
			image = NULL;
			opacity = 1.0;
		}

		virtual ~GEDrawElement() {
			if( image != NULL )
				delete image;
		}

		Image *image;
		unsigned int imageid;
		GuiDim x, y, 
			stretchFactorx, //how much to stretch if not ST_NONE
			stretchFactory;
		StretchType stretchType;

		float opacity;	//tranparency of draw
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
		GUITextures( std::string fileName ) { }

		GUITextures ( ) {}
		virtual ~GUITextures() {}

		void loadFile( std::string fileName) {
			loadFromFile( fileName );
			iter = fileList.begin();

			if( fileList.size() == 0 ) {
				throw GUITexturesLoadException( "GUITextures::loadFile", "No textures in list" );
			}
		}

		//TODO validation to make sure there is another texture to output
		//TODO rewrite using c++ stuff
		const char *getNextTexture() {
			const char *returnStr;
			std::string str;
			str = ( ( std::string )*iter++ );
			returnStr = str.c_str();
			
			#if _MSC_VER > 1000
				strncpy_s( buffer, str.c_str(), str.size() );
			#else 
				strncpy( buffer, str.c_str(), str.size() );
			#endif
			
			return buffer;
		}

		//initialize exception class
		class GUITexturesLoadException {
		  public:
			GUITexturesLoadException( string header, string m, bool s = true ) {
				::CgtException( header, m, s );
			}
		};
		
		
	private:
		std::list< std::string > fileList;
		std::list< std::string >::iterator iter;
		std::string fileName;
		void loadFromFile( std::string fileName );
		void parseLine( std::string line );

		char buffer[ 255 ];

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
		GUIElement() {}	
		GUIElement( std::string guitextures );

		virtual ~GUIElement();

		//these "events" are called when certain mouse actions are performed by the user
		virtual void onLeftMouseDown() {setAsActiveElement();}
		virtual void onLeftMouseRelease() {}
		virtual void onMouseOver() {}
		virtual void onRightMouseDown() {}
		virtual void onRightMouseRelease() {}

		//called when user presses a key on keyboard while a control is selected
		virtual void onKeyDown( unsigned short key ) {}
		
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
			children.push_front( elem );
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
			GUIElementInitializeException( string header, string m, bool s = true ) {
				::CgtException( header, m, s );
			}
		};

		friend class GUI;

	protected:
		//every gui element will have 0 or more children.  Children are guielements
		//contained within this gui element
		std::list< GUIElement* > children;

		//list of draw elements
		std::list< GEDrawElement* >drawList;

		//gui textures object
		GUITextures gt;

		GuiDim x, y, width, height;
		int type;
		
		bool activeElement;
		bool open;

		//if this element is a child to another gui element than this will point to
		//the parent gui element
		GUIElement *parent;

		virtual void initialize( std::string guitextures );	
		
		void unactivateChildren( std::list< GUIElement* > childList );
		GUIElement *getActiveChild( std::list< GUIElement* > childList );
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
	};

	class Font {
	public:
		Font();
		~Font();

		void loadFont( const char *file );

		GuiDim &getFontWidth() {return fontWidth;}
		GuiDim &getFontHeight() {return fontHeight;}

		Image *geImage() {return image;}

		//pixel coordinates to draw given character at
		//must call setChar first to get these
		TextureCoord *getCoord( char c );

		unsigned int geImageid() {return imageid;}

	private:
		char currChar;
		GuiDim fontWidth;
		GuiDim fontHeight;
	

		//builds the texture coordinate lookup table
		void buildCoordTable();

		//current coordinates
		TextureCoord currCoord;

		vector< TextureCoord > coordTable;

		Image *image;
		unsigned int imageid;
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

		void onKeyDown( unsigned char key ) {}

		void update() {}

		void loadFont( Font *font ) {this->font = font;}
		
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

		void onKeyDown( unsigned char key ) {}

		void update() {}

		unsigned int getNumLines() { return lines.size(); }

		void addLines( int numLines );

		void setX( GuiDim val );
		void setY( GuiDim val );

		//fills text labels with multi-lined string "in"
		bool fillLines( string in );
	private:
		vector< TextLabel* > lines;
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

		virtual void onKeyDown( unsigned char key ) {}

		void update();

		std::string getText() {return text;}
		void setText( const char *text );
		void setText( std::string text );
		void clearText() {
			text = "";
			boxtext.setString( "" );
			cursorPos = 0;
			viewPos = 0;
		}

		void setCursorPos( int pos ) {cursorPos = pos;}
		void setMaxLength( int l ) { maxLength = l; }
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
			void onLeftMouseDown() {parent->onLeftMouseDown();}
		}boxtext;
		
		Font font;

		void initialize( std::string guitextures );
	};

	/*
	=================================================================
	=================================================================
	Button Class
	=================================================================
	=================================================================
	*/
	class Button : public GUIElement {
	public:	
		Button();
		Button( std::string guitextures );
		~Button();
		
		void onLeftMouseDown();
		void onLeftMouseRelease();
		void onMouseOver();
		void onRender();
		void onRightMouseDown();
		void onRightMouseRelease();

		virtual void onButtonPress();

		void update();
	protected:
		GEDrawElement l, r, b, t, bl, br, tl, tr;			//left , right, bottom, etc..
		GEDrawElement dl, dr, db, dt, dbl, dbr, dtl, dtr;	//for when button is down

		bool buttonDown;
		
		void initialize( std::string guitextures );
	};

	class CloseButton : public Button {
		void onButtonPress();
		
	};

	/*
	=================================================================
	=================================================================
	TitleBar Class
	=================================================================
	=================================================================
	*/
	class TitleBar : public GUIElement {
	public:
		TitleBar();
		TitleBar( std::string guitextures );
		~TitleBar();

		void onLeftMouseDown();
		void onLeftMouseRelease();
		void onMouseOver();
		void onRender();
		void onRightMouseDown();
		void onRightMouseRelease();

		void update();

		void setDialogTitle( DialogTitle *dt ) {
			textLabel = dt;
			addChild( textLabel );
		}

		friend class DialogBox;

	protected:
		GEDrawElement left, right, middle;

		int lastx, lasty;
		bool movement;

		DialogTitle *textLabel;
		CloseButton *closeButton;

		void initialize( std::string guitextures );
	};



	/*
	=================================================================
	=================================================================
	DialogBox Class
	=================================================================
	=================================================================
	*/
	class DialogBox : public GUIElement {
	public:
		DialogBox();
		DialogBox( std::string guitextures );
		~DialogBox();

		void onLeftMouseDown();
		void onLeftMouseRelease();
		void onMouseOver();
		void onRender();
		void onRightMouseDown();
		void onRightMouseRelease();
		void update() {}

		void setTitle( char *str ) {title->setString( str );}

	protected:
		TitleBar *titleBar;

		DialogTitle *title;
		Font *font;
		
		GEDrawElement lborder, rborder, blcorner, brcorner, bborder, background;

		void initialize( std::string guitextures );
	};

	/*
	=================================================================
	=================================================================
	Slider Class
	=================================================================
	=================================================================
	*/
	#define maxSliderHeight 0x666666

	class Slider : public GUIElement {
	public:
		//type = SLIDER_X or SLIDER_Y - used to choose whether slider moves up and down
		//								or left and right
		Slider( int type );
		Slider( std::string guitextures, int type );
		
		~Slider();

		void onLeftMouseDown();
		void onLeftMouseRelease();
		void onMouseOver();
		void onRender();
		void onRightMouseDown();
		void onRightMouseRelease();

		void update();

		//get current value of slider ( 0x0 - maxSliderHeight ) 
		int getValue() {return value;}
		
		//if set to true the value will be displayed beside the slider
		void setDrawValueLabel( bool val ) {drawValueLabel = val;valueLabel.setOpen( drawValueLabel );}

		GuiDim calcSliderPos();

		friend class SliderBar;
		enum {
			SLIDER_X,
			SLIDER_Y
		};

	protected:

		GEDrawElement top, bottom, middle;
		Font font;
		TextLabel valueLabel;

		bool slideControl;
		unsigned int value;			
		int type;					

		int velocity;
		int mouseX, mouseY;
				
		bool drawValueLabel;

		void initialize( std::string guitextures );

		//controls the slider bar on the slider
		class SliderBar : public GUIElement {
		public:	
			SliderBar( int type );
			SliderBar( std::string guitextures, int type );
			~SliderBar();

			void onLeftMouseDown();
			void onLeftMouseRelease();
			void onMouseOver();
			void onRender();
			void onRightMouseDown();
			void onRightMouseRelease();

			void update();
		private:
			GEDrawElement sliderBar;
			int type;

			void initialize( std::string guitextures );
		}*sliderBar;
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

		//updates all elements based on user's actions
		void runFrame();
		
		//renders ui
		void render();
		
		//adds a gui element to the gui system
		void addElement( GUIElement *ge ) {elements.push_front( ge );}

		//sets whether the gui is using the mouse
		void setUsingMouse( bool val ) {usingMouse = val;}
		
		//is the gui currently using the mouse?
		bool isUsingMouse() {return usingMouse;}

		//unactivates all elements in the elements list
		void unactivateAllElements();

		//initialize exception class
		class GUIRunException : public CgtException {
		public:
			GUIRunException( string header, string m, bool s = true ) {
				::CgtException( header, m, s );
			}
		};
		GUI();
		virtual ~GUI();

	private:
		std::list< GUIElement* > elements;
		
		bool usingMouse;
		//bool usingKeyboard;

		//used so elements don't overlap when drawn overtop of eachother
		float zdrawpos;

		void updateElementsList( std::list< GUIElement* > elems );
		void renderElements( std::list< GUIElement* > elems );
		void renderTextLabel( TextLabel *fs );
		void renderDebugLines( GUIElement *element );
		void renderDrawList( std::list< GEDrawElement* > drawList );
		GUIElement *findElementCursorOver();
		void sendActiveToFront();

		//bool drawDebugLines;
		bool drawGUI;
	};

}	//namespace gui
#endif // !defined( AFX_GUI_H__B172BAD5_6BBC_442E_B396_1207A4FAFF65__INCLUDED_ )
