// Input.cpp: implementation of the Input class.
//
//////////////////////////////////////////////////////////////////////
#include "precompiled.h"

#include "Input.h"
#include "Console.h"

#include <exception>
#include "imgui/imgui.h"

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
	return nullptr;
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

ControllableButton::ControllableButton() :
	ControllableButton("NOT_SET", "NOT_SET", "NOT_SET" )
{}

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
	mousex(0),
	mousey(0),
	screenx(0),
	screeny(0),
	useDelay( false ),
	lastKeyPressed( 0 ),
	kbpos( 0 ),
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
		//state.second = false;
	}

	for ( auto& state : axisState ) {
		//state.second = 0;
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
void Input::clearInputState()
==============================================
*/
void Input::clearInputState( ) {
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
std::ostringstream controlLog;
void Input::updateControllables() {
	for( unsigned int c = 0 ; c < controllables.size(); c++ ) {
		for( unsigned int b = 0 ; b < controllables[ c ]->buttons.size(); b++ ) {
			auto& button = controllables[ c ]->buttons[ b ];
			//handle keyboard
			if ( button->deviceName == "keyboard" ) {
				if ( inputState == Input::InputSystemStates::TYPE_MODE ) {
					break;
				}
				//get key id from string
				auto keyid = keyStringToNumber( button->bindName );
				button->keystate = ( KeyPressState )keystate[ keyid ];
			} else {
				//handle joystick/gamepad
				auto& deviceName = button->deviceName;	//USB_Gamepad
				
				if ( deviceName.empty( ) ) {
					break;
				}
				auto& bindName = button->bindName;		//1

				//get information from input datastructure
				button->keystate = ( KeyPressState )getButtonState( deviceName, bindName );
				if( button->keystate ) {
					controlLog << "setting:" << button->keystate << " deviceName: " << deviceName << " to " << " bind name: " << bindName
						<< std::endl;
				}
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
==============================================
*/
bool Input::bindKeyToControl( std::string_view device, std::string_view keystr, std::string_view controlstr, std::string_view buttonstr ) {
	//validation
	//TODO Exception handler
		//consoleSystem->printMessage( "Input::bindKeyToControl - no key is bound" );
		return false;
	}

	//find control
	Controllable *control = getControl( controlstr );

	//if control not found return error
	if( control == nullptr ) {
		consoleSystem->printMessage( "Input::bindKeyToControl - control not found" );
		return false;
	}

	//find button
	ControllableButton *button = control->getButton( buttonstr );

	//if button not found return error
	if( button == nullptr ) {
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
	return nullptr;
}

/*
==============================================
Input::generateControllerBindCommand
==============================================
*/
std::string Input::generateControllerBindCommand( const Controllable& controllable, const ControllableButton& button ) {
	std::string controllerName = controllable.name + "." + button.name;
	std::string bindName = button.deviceName + "." + button.bindName;

	return "bind " + controllerName + " to " + bindName;
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
			std::string bindCommand = generateControllerBindCommand( *controllable, *button );
			file << bindCommand << std::endl;
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
	for ( auto& j : joysticks ) {
		auto& joystick = j.second;
		if ( deviceName == joystick.name ) {
			//handle axis
			if ( bindName.length() >= 4 && bindName.substr( 0, 4 ) == "axis" ) {
				int axisNum = bindName[ 4 ] - '0';
				bool min = bindName.substr( 5, 4 ) == "min";
				bool max = bindName.substr( 5, 4 ) == "max";
				int axisValue = joystick.axisState[ axisNum ];
				
				if( min && axisValue < -10000 ) {
					return true;
				}
				if ( max && axisValue > 10000 ) {
					return true;
				}

				return false;
			}

			//handle button bind
			int bindNum = std::stoi( bindName.data() );
			bool buttonState = joystick.buttonState[ bindNum ];
			
			return buttonState;
		}
	}
	return false;
}