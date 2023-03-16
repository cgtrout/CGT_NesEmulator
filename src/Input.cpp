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
ControllableButton::ControllableButton( std::string_view name, std::string_view deviceName, std::string_view bindName ) :
	name(name), 
	deviceName(deviceName),
	bindName(bindName ),
	keystate(NOT_PRESSED)
{
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
Joystick::clearState( )
==============================================
*/
void Joystick::clearState( ) {
	//reset all bound buttons to false (not pressed state)
	for ( auto& state : buttonState ) {
		state.second = false;
	}
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
	kbpos = 0;
	lastKeyPressed = 0;

	clearInputState( );
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
void Input::clearInputState()
==============================================
*/
void Input::clearInputState() {
	keystate.clear( );

	for ( auto& j : joysticks ) {
		j.clearState( );
	}
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
	for( unsigned int c = 0 ; c < controllables.size(); c++ ) {
		for( unsigned int b = 0 ; b < controllables[ c ]->buttons.size(); b++ ) {
			auto& button = controllables[ c ]->buttons[ b ];
			if ( button->deviceName == "keyboard" ) {
				if ( inputState == Input::InputSystemStates::TYPE_MODE ) {
					return;
				}
				//get key id from string
				auto keyid = keyStringToNumber( button->bindName );
				button->keystate = ( KeyPressState )keystate[ keyid ];
			} else {
				auto& deviceName = button->deviceName;	//USB_Gamepad
				
				if ( deviceName.empty( ) ) {
					return;
				}
				auto& bindName = button->bindName;		//1

				//need to somehow get information from input datastructure
				button->keystate = ( KeyPressState )input->getButtonState( deviceName, bindName );
				
			}
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
bool Input::bindKeyToControl( std::string_view device, std::string_view keystr, std::string_view controlstr, std::string_view buttonstr ) {
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
	button->deviceName = device;
	button->bindName = keystr;
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
			auto& controllable = controllables[ c ];
			auto& button = controllable->buttons[ b ];
			file << "bind " << controllables[ c ]->name.c_str() << ".";
			file << button->name.c_str() << " to ";
			
			file << "keyboard.";
			//dont output if keybind = 0
			if ( button->bindName.empty() == false ) {
				file << button->bindName;
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

/*
==============================================
Input::getButtonState
==============================================
*/
bool Input::getButtonState( std::string_view deviceName, std::string_view bindName ) {
	//loop to find controller
	for ( auto& joystick : joysticks ) {
		if ( deviceName == joystick.name ) {
			//for now convert bindName to a int
			int bindNum = std::stoi( bindName.data() );
			bool buttonState = joystick.buttonState[ bindNum ];
			if ( buttonState ) {
				int i = 0;
			}
			
			return buttonState;
		}
	}
	return false;
}