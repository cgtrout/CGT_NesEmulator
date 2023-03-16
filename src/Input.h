// Input.h: interface for the Input class.
//
//////////////////////////////////////////////////////////////////////

#if !defined( AFX_INPUT_H__87C1253D_A468_45F7_B983_6F72433ABB39__INCLUDED_ )
#define AFX_INPUT_H__87C1253D_A468_45F7_B983_6F72433ABB39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <SDL_keyboard.h>

#include "typedefs.h"

#include "CGTSingleton.h"
#include <map>
#include <string_view>
#include <vector>
#include <string>
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
			ControllableButton( std::string_view name,
								std::string_view deviceName,
								std::string_view bindName, int keyid );
			friend class Controllable;

			std::string name;			//name of this button
			std::string deviceName;		//name of device bound to this
			std::string bindName;		//name of button or keybind bound to this

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
			Controllable( std::string_view name );
			Controllable() { }

			void addButton( ControllableButton *b );
			std::string name;
			
			//get a button from controls list of buttons
			//returns null if not found
			ControllableButton *getButton( std::string_view name );

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
			
			enum class InputSystemStates { 
				TYPE_MODE,		// user is in type control
				NORMAL_MODE		// 
			};
			
			Input();
			virtual ~Input();

			void render();
			void init();
			void clear();

			InputSystemStates getState() { return inputState; }
			void setState( InputSystemStates s );
			
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

			void clearKeyState();

			//add text to textInput (for typing input)
			void addTextInput( const std::string input ) { textInput += input; }

			//reset/clear text input
			void clearTextInput( ) { textInput.clear( ); }

			std::string getTextInput( ) { return textInput; }

			bool isKeyDown( SDL_Keycode key ) { return keystate[ key ]; }
			
			//unsets they key as it reads it
			bool isKeyDownUnset( SDL_Keycode key );
			
			void setKeyDown( SDL_Keycode key ) { keystate[ key ] = true; }
			void setKeyUp( SDL_Keycode key ) { keystate[ key ] = false; lastKeyPressed = key; }

			void setUseDelay( bool val ) { useDelay = val; }

			SDL_Keycode getLastKeyPressed() { return lastKeyPressed; }

			//adds control to input system 
			void addControl( Controllable* );

			//update all controllables connected to input system
			void updateControllables();

			//binds a key to a control
			bool bindKeyToControl( std::string_view device, std::string_view key, std::string_view control, std::string_view button );

			//get control from list of controls
			Controllable *getControl( std::string_view control );

			//writes all binds to a file
			void writeBindsToFile( std::string_view filename );

			//convert keyid to it's std::string form
			std::string keyIdToString( SDL_Keycode keyid );

			//convert a key in string form to it's id number
			SDL_Keycode keyStringToNumber( std::string_view key );
			
		private:

			InputSystemStates inputState;
			std::map<SDL_Keycode, bool>  keystate;
			int kbpos;			//keybuffer position
			bool useDelay;

			SDL_Keycode lastKeyPressed;

			std::string textInput;
				
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

#endif // !defined( AFX_INPUT_H__87C1253D_A468_45F7_B983_6F72433ABB39__INCLUDED_ )
