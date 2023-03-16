// Input.cpp: implementation of the Input class.
//
//////////////////////////////////////////////////////////////////////
#include "precompiled.h"

#include "Input.h"
#include "Console.h"

#include <exception>

using namespace FrontEnd::InputSystem;

/* 
==============================================
Controllable::Controllable()
==============================================
*/
Controllable::Controllable( std::string_view name ) : name( name ) {

}

/* 
==============================================
void Controllable::addButton( ControllableButton *b )
==============================================
*/
void Controllable::addButton( ControllableButton *b ) {
	buttons.push_back( b );
}

/* 
==============================================
ControllableButton *Controllable::getButton
==============================================
*/
ControllableButton *Controllable::getButton( std::string_view name ) {
	for( unsigned int i = 0; i < buttons.size(); i++ ) {
		if( buttons[ i ]->name == name ) {
			return buttons[ i ];
		}
	}
	return NULL;
}

/* 
==============================================
ControllableButton::ControllableButton()
==============================================
*/
ControllableButton::ControllableButton( std::string_view name, int keyid ) {
	this->name = name;
	this->keyid = keyid;
	keystate = NOT_PRESSED;
}

/*
==============================================
Input::Input()
==============================================
*/
Input::Input(): 
	mouseLeftDown( false ), 
	mouseLeftUp( false ), 
	mouseRightDown( false ), 
	mouseRightUp( false ),
	mouseLeftDownLastFrame( false ),
	mouseRightDownLastFrame( false ),
	useDelay( false ),
	lastKeyPressed( 0 ),
	inputState( InputSystemStates::NORMAL_MODE ) { 
}

/*
==============================================
Input::~Input()
==============================================
*/
Input::~Input() {
	//writeBindsToFile( "binds.cfg" );
}

/*
==============================================
Input::render()
==============================================
*/
void Input::render() {
		
}

/*
==============================================
Input::init()
==============================================
*/
void Input::init() {
	
}

/*
==============================================
void Input::clear()
==============================================
*/
void Input::clear() {
	mouseLeftDown = false;
	mouseLeftUp = false;
	mouseRightDown = false;
	mouseRightUp = false;
	//clearKeyBuffers();
	if( inputState == InputSystemStates::TYPE_MODE ) {
		clearKeyState();
	}
	kbpos = 0;
	lastKeyPressed = 0;
}

/*
==============================================
Input::setState
==============================================
*/
void Input::setState( InputSystemStates s ) { 
	inputState = s; 

	switch ( inputState ) {
		case InputSystemStates::TYPE_MODE:
			SDL_StartTextInput( );
			break;
		case InputSystemStates::NORMAL_MODE:
			SDL_StopTextInput( );
			clearTextInput( );
			break;
	}

}

/*
==============================================
void Input::clearKeyState()
==============================================
*/
void Input::clearKeyState() {
	keystate.clear( );
}

/*
==============================================
bool Input::isKeyDownUnset
	unsets the key as it returns the state
==============================================
*/
bool Input::isKeyDownUnset( SDL_Keycode key ) {
	bool s = keystate[ key ];
	keystate[ key ] = false;
	return s;
}

/* 
==============================================
void Input::updateControllables()

  goes through all controllables ( and their associated buttons ) and
  updates their key press states
==============================================
*/
void Input::updateControllables() {
	// check to see if input is currently using input to fill a gui control 
	if( inputState == Input::InputSystemStates::TYPE_MODE ) {
		return;
	}
	for( unsigned int c = 0 ; c < controllables.size(); c++ ) {
		for( unsigned int b = 0 ; b < controllables[ c ]->buttons.size(); b++ ) {
			controllables[c]->buttons[b]->keystate = (KeyPressState)keystate[ controllables[c]->buttons[b]->keyid ];
		}
	}
}

/* 
==============================================
void Input::addControl( Controllable *control )
==============================================
*/
void Input::addControl( Controllable *control ) {
	controllables.push_back( control );
}

/* 
==============================================
bool Input::bindKeyToControl
TODO way for user to find out what key a control's button is bound to
	 way for user to find out what buttons are in a particular command
	 way for user to find out what commands are in input system
==============================================
*/
bool Input::bindKeyToControl( std::string_view keystr, std::string_view controlstr, std::string_view buttonstr ) {
	//validation
	//TODO Exception handler
	if( keystr == "NULL" ) {
		//consoleSystem->printMessage( "Input::bindKeyToControl - no key is bound" );
		return false;
	}
	
	SDL_Keycode keyid = keyStringToNumber( keystr );
	if( keyid == NULL ) {
		consoleSystem->printMessage( "Input::bindKeyToControl - key doesn't exist" );
		return false;
	}

	//find control
	Controllable *control = getControl( controlstr );

	//if control not found return error
	if( control == NULL ) {
		consoleSystem->printMessage( "Input::bindKeyToControl - control not found" );
		return false;
	}

	//find button
	ControllableButton *button = control->getButton( buttonstr );

	//if button not found return error
	if( button == NULL ) {
		consoleSystem->printMessage( "Input::bindKeyToControl - button not found" );
		return false;
	}
	
	//everything looks good - now assign the new bind
	button->keyid = keyid;
	return true;
}

/* 
==============================================
Controllable *Input::getControl
==============================================
*/
Controllable *Input::getControl( std::string_view control ) {
	//find control
	for( unsigned int x = 0 ; x < controllables.size(); x++ ) {
		if( controllables[ x ]->name == control ) {
			return controllables[ x ];
		}
	}
	return NULL;
}

/* 
==============================================
void Input::writeBindsToFile
==============================================
*/
void Input::writeBindsToFile( std::string_view filename ) {
	std::ofstream file( filename.data() );
	file << "//Keybinds file - modify binds only.  Everything else is not preserved \n";
	
	//go through all controls
	for( unsigned int c = 0; c < controllables.size(); c++ ) {
		//go through all buttons on current control
		for( unsigned int b = 0; b < controllables[ c ]->buttons.size(); b++ ) {
			file << "bind " << controllables[ c ]->name.c_str() << ".";
			file << controllables[ c ]->buttons[ b ]->name.c_str() << " to ";
			
			//dont output if keybind = 0
			if ( controllables[ c ]->buttons[ b ]->keyid != 0 ) {
				SDL_Keycode keyId = controllables[ c ]->buttons[ b ]->keyid;
				auto keyString = keyIdToString( keyId );
				file << keyString;
			}
			file << """\n";
		}
	}
	file << std::endl;
	file.close();
}

/* 
==============================================
Input::keyIdToString
==============================================
*/
std::string Input::keyIdToString( SDL_Keycode keyid ) {
	std::string retValue( SDL_GetKeyName( keyid ) );
	return retValue;
}

/* 
==============================================
uword Input::keyStringToNumber
==============================================
*/
SDL_Keycode Input::keyStringToNumber( std::string_view key ) {	
	SDL_Keycode ret = SDL_GetKeyFromName( key.data() );
	if ( ret == SDLK_UNKNOWN ) {
		throw std::invalid_argument( "Input: KeyStringToNumber - invalid" );
	}

	return ret;
}