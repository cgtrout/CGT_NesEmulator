// Input.h: interface for the Input class.
//
//////////////////////////////////////////////////////////////////////

#if !defined( AFX_INPUT_H__87C1253D_A468_45F7_B983_6F72433ABB39__INCLUDED_ )
#define AFX_INPUT_H__87C1253D_A468_45F7_B983_6F72433ABB39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include < windows.h >	
#include < vector >
#include < string >
using namespace std;


#include "CGTSingleton.h"
using namespace CGTSingleton;


namespace FrontEnd {
	namespace InputSystem {
		enum KeyPressState {
			PRESSED = 1,
			NOT_PRESSED = 0
		};
		
		/*
		================================================================
		================================================================
		Class
		ControllableButton - button that can be pressed / not pressed
		================================================================
		================================================================
		*/
		class ControllableButton {
		 friend class Input;
		 public:
			ControllableButton() { }
			ControllableButton( char *name, int keyid );
			friend class Controllable;

			uword keyid;
			std::string name;

			KeyPressState getState() { return keystate; }
			
			void  setState( KeyPressState s ) { keystate = s; }
		private:
			KeyPressState keystate;
		};

		/*
		================================================================
		================================================================
		Class
		Controllable
		  represents a controllable class - this means that the class
		  has one or more buttons that it controls.  All Controllable
		  classes must be added to the master Input list by using
		  Input::addControl( *Controllable )
		================================================================
		================================================================
		*/
		class Controllable {
		  friend class Input;
		  public:
			Controllable( char *name );
			Controllable() { }

			void addButton( ControllableButton *b );
			std::string name;
			
			//get a button from controls list of buttons
			//returns null if not found
			ControllableButton *getButton( std::string name );

			//call once per frame to clear all button states
			void clearStates();
		  private:
			std::vector< ControllableButton* > buttons;
		};

		/*
		================================================================
		================================================================
		Class Input : public Singleton< Input >

		  main input class - handles all keyboard and mouse input
		================================================================
		================================================================
		*/
		class Input : public Singleton< Input > {
		public:
			friend Singleton< Input >;
			
			enum InputSystemStates { 
				TYPE_MODE,		// user is in type control
				NORMAL_MODE		// 
			};
			
			Input();
			virtual ~Input();

			void render();
			void init();
			void clear();

			InputSystemStates getState() { return inputState; }
			void setState( InputSystemStates s ) { inputState = s; }
			
			//mouse button state getters
			bool isMouseLeftDown() {return mouseLeftDown;}
			bool isMouseLeftUp() {return mouseLeftUp;}
			bool isMouseRightDown() {return mouseRightDown;}
			bool isMouseRightUp() {return mouseRightUp;}

			//used to set mouse state
			void setMouseLeftDown( bool val ) {mouseLeftDown = val;}
			void setMouseLeftUp( bool val ) {mouseLeftUp = val;}
			void setMouseRightDown( bool val ) {mouseRightDown = val;}
			void setMouseRightUp( bool val ) {mouseRightUp = val;}

			void setMouseLeftDownLastFrame( bool val ) {mouseLeftDownLastFrame = val;}
			bool wasMouseLeftDownLastFrame() {return mouseLeftDownLastFrame;}

			int getMouseX() {return mousex;}
			int getMouseY() {return mousey;}

			void setMouseX( int val ) {mousex = val;}
			void setMouseY( int val ) {mousey = val;}

			void setScreenX( int val ) {screenx = val;}
			void setScreenY( int val ) {screeny = val;}

			//moves mouse cursor to coordinates set by setMouseX and setMouseY
			void moveMouseToNewCoordinates() {SetCursorPos( mousex + screenx, mousey + screeny );}

			//key buffer functions
			//adds key to buffer ( used for typeing )
			void addKeyToBuffer( unsigned char key ) {keybuffer[ keybuffersize++ ] = key;}
			
			char getNextKeyMessage() { return keybuffer[ kbpos++ ]; }
			bool isMoreKeyMessages() { return kbpos < keybuffersize; }
			void clearKeyState();

			bool isKeyDown( uword key ) { return keystate[ key ]; }
			
			//unsets they key as it reads it
			bool isKeyDownUnset( uword key );
			
			void setKeyDown( uword key ) { keystate[ key ] = true; }
			void setKeyUp( uword key ) { keystate[ key ] = false; lastKeyPressed = key; }

			void setUseDelay( bool val ) { useDelay = val; }

			uword getLastKeyPressed() { return lastKeyPressed; }

			//adds control to input system 
			void addControl( Controllable* );

			//update all controllables connected to input system
			void updateControllables();

			//binds a key to a control
			bool bindKeyToControl( string key, string control, string button );

			//get control from list of controls
			Controllable *getControl( string control );

			//writes all binds to a file
			void writeBindsToFile( const char *filename );

			//convert keyid to it's string form
			string keyIdToString( uword keyid );

			//convert a key in string form to it's id number
			uword keyStringToNumber( string key );
			
		private:

			InputSystemStates inputState;
			//todo extend keystate system to handle external
			//controllers through direct input			
			bool keystate[ 1024 ];
			int kbpos;			//keybuffer position
			bool useDelay;

			uword lastKeyPressed;

			unsigned char keybuffer[ 100 ];
			int keybuffersize;
				
			bool mouseLeftDown;
			bool mouseLeftUp;
			bool mouseRightDown;
			bool mouseRightUp;

			bool mouseLeftDownLastFrame;
			bool mouseRightDownLastFrame;

			int mousex; 
			int mousey;

			int screenx;	//screen coordinates - used for mouse positioning
			int screeny;

			//list of all controllable classes
			std::vector< Controllable* > controllables;
		};
	}
}

//keyboard key definitions - set to directly correspond to windows virtual keys
#define KB_BACK           0x08
#define KB_TAB            0x09

#define KB_CLEAR          0x0C
#define KB_RETURN         0x0D

#define KB_SHIFT          0x10
#define KB_CONTROL        0x11
#define KB_MENU           0x12
#define KB_PAUSE          0x13
#define KB_CAPITAL        0x14

#define KB_ESCAPE         0x1B

#define KB_SPACE          0x20

//THESE TWO KEYS ARE NOT SPECIFIED IN WINDOWS DEFINES
#define KB_PGUP	          0x21
#define KB_PGDOWN         0x22

#define KB_END            0x23
#define KB_HOME           0x24
#define KB_LEFT           0x25
#define KB_UP             0x26
#define KB_RIGHT          0x27
#define KB_DOWN           0x28
#define KB_SNAPSHOT       0x2C
#define KB_INSERT         0x2D
#define KB_DELETE         0x2E

/* KB_0 thru KB_9 are the same as ASCII '0' thru '9' ( 0x30 - 0x39 ) */
#define KB_0			  0x30
#define KB_1			  0x31
#define KB_2			  0x32
#define KB_3			  0x33
#define KB_4			  0x34
#define KB_5			  0x35
#define KB_6			  0x36
#define KB_7			  0x37
#define KB_8			  0x38
#define KB_9			  0x39

/* KB_A thru KB_Z are the same as ASCII 'A' thru 'Z' ( 0x41 - 0x5A ) */

#define KB_A			  0x41
#define KB_B			  0x42
#define KB_C			  0x43
#define KB_D			  0x44
#define KB_E			  0x45
#define KB_F			  0x46
#define KB_G			  0x47
#define KB_H			  0x48
#define KB_I			  0x49
#define KB_J			  0x4A
#define KB_K			  0x4B
#define KB_L			  0x4C
#define KB_M			  0x4D
#define KB_N			  0x4E
#define KB_O			  0x4F
#define KB_P			  0x50
#define KB_Q			  0x51
#define KB_R			  0x52
#define KB_S			  0x53
#define KB_T			  0x54
#define KB_U			  0x55
#define KB_V			  0x56
#define KB_W			  0x57
#define KB_X			  0x58
#define KB_Y			  0x59
#define KB_Z			  0x5A

#define KB_NUMPAD0        0x60
#define KB_NUMPAD1        0x61
#define KB_NUMPAD2        0x62
#define KB_NUMPAD3        0x63
#define KB_NUMPAD4        0x64
#define KB_NUMPAD5        0x65
#define KB_NUMPAD6        0x66
#define KB_NUMPAD7        0x67
#define KB_NUMPAD8        0x68
#define KB_NUMPAD9        0x69
#define KB_MULTIPLY       0x6A
#define KB_ADD            0x6B
#define KB_SEPARATOR      0x6C
#define KB_SUBTRACT       0x6D
#define KB_DECIMAL        0x6E
#define KB_DIVIDE         0x6F
#define KB_F1             0x70
#define KB_F2             0x71
#define KB_F3             0x72
#define KB_F4             0x73
#define KB_F5             0x74
#define KB_F6             0x75
#define KB_F7             0x76
#define KB_F8             0x77
#define KB_F9             0x78
#define KB_F10            0x79
#define KB_F11            0x7A
#define KB_F12            0x7B
#define KB_F13            0x7C
#define KB_F14            0x7D
#define KB_F15            0x7E
#define KB_F16            0x7F
#define KB_F17            0x80
#define KB_F18            0x81
#define KB_F19            0x82
#define KB_F20            0x83
#define KB_F21            0x84
#define KB_F22            0x85
#define KB_F23            0x86
#define KB_F24            0x87

#define KB_TILDE		  0xC0

#endif // !defined( AFX_INPUT_H__87C1253D_A468_45F7_B983_6F72433ABB39__INCLUDED_ )
