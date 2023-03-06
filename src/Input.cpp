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
Controllable::Controllable( char *name ) : name( name )
==============================================
*/
Controllable::Controllable( char *name ) : name( name ) {

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
ControllableButton *Controllable::getButton( std::string name )
==============================================
*/
ControllableButton *Controllable::getButton( std::string name ) {
	for( unsigned int i = 0; i < buttons.size(); i++ ) {
		if( buttons[ i ]->name == name ) {
			return buttons[ i ];
		}
	}
	return NULL;
}

/* 
==============================================
ControllableButton::ControllableButton( char *name, int keyid )
==============================================
*/
ControllableButton::ControllableButton( char *name, int keyid ) {
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
	inputState( NORMAL_MODE ) { 
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
	if( inputState == TYPE_MODE ) {
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
		case TYPE_MODE:
			SDL_StartTextInput( );
			break;
		case NORMAL_MODE:
			SDL_StopTextInput( );
			break;
	}

}

/*
==============================================
void Input::clearKeyState()
==============================================
*/
void Input::clearKeyState() {
	for( int x = 0; x < SDL_NUM_SCANCODES; x++ ) {
		keystate[ x ] = false;
	}	
}

/*
==============================================
bool Input::isKeyDownUnset( uword key )
	unsets the key as it retursn the state
==============================================
*/
bool Input::isKeyDownUnset( uword key ) { 
	bool ks = keystate[key];
	keystate[key] = 0; 
	return ks; 
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
	if( inputState == TYPE_MODE ) {
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
bool Input::bindKeyToControl( std::string key, string control, string button )
TODO way for user to find out what key a control's button is bound to
	 way for user to find out what buttons are in a particular command
	 way for user to find out what commands are in input system
==============================================
*/
bool Input::bindKeyToControl( std::string keystr, std::string controlstr, std::string buttonstr ) {
	//validation
	//TODO Exception handler
	if( keystr == "NULL" ) {
		//consoleSystem->printMessage( "Input::bindKeyToControl - no key is bound" );
		return false;
	}
	
	uword keyid = keyStringToNumber( keystr );
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
Controllable *Input::getControl( std::string control )
==============================================
*/
Controllable *Input::getControl( std::string control ) {
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
void Input::writeBindsToFile()
==============================================
*/
void Input::writeBindsToFile( const char *filename ) {
	std::ofstream file( filename );
	file << "//Keybinds file - modify binds only.  Everything else is not preserved \n";
	
	//go through all controls
	for( unsigned int c = 0; c < controllables.size(); c++ ) {
		//go through all buttons on current control
		for( unsigned int b = 0; b < controllables[ c ]->buttons.size(); b++ ) {
			file << "bind " << controllables[ c ]->name.c_str() << ".";
			file << controllables[ c ]->buttons[ b ]->name.c_str() << " to ";
			
			//dont output if keybind = 0
			if ( controllables[ c ]->buttons[ b ]->keyid != 0 ) {
				file << keyIdToString( controllables[ c ]->buttons[ b ]->keyid );
			}
			file << """\n";
		}
	}
	file << std::endl;
	file.close();
}

std::string Input::keyIdToString( uword keyid ) {
	return std::string(SDL_GetKeyName( keyid ));
}

/* 
==============================================
uword Input::keyStringToNumber
==============================================
*/
uword Input::keyStringToNumber( std::string_view key ) {	
	ubyte ret = SDL_GetScancodeFromName( key.data() );
	if ( ret == SDL_SCANCODE_UNKNOWN ) {
		throw std::invalid_argument( "Input: KeyStringToNumber - invalid" );
	}

	return ret;
}