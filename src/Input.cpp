// Input.cpp: implementation of the Input class.
//
//////////////////////////////////////////////////////////////////////
#include "precompiled.h"

#include "Input.h"
#include "Console.h"

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
	keybuffersize( 0 ),
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
	writeBindsToFile( "binds.cfg" );
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
	keybuffersize = 0; 
	kbpos = 0;
	lastKeyPressed = 0;
}

/*
==============================================
void Input::clearKeyState()
==============================================
*/
void Input::clearKeyState() {
	for( int x = 0; x < 512; x++ ) {
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
			file << keyIdToString( controllables[ c ]->buttons[ b ]->keyid ) << "\n";
		}
	}
	file << std::endl;
	file.close();
}

std::string Input::keyIdToString( uword keyid ) {
	if( keyid == KB_BACK     ) return "back";
	if( keyid == KB_TAB      ) return "tab";
	if( keyid == KB_CLEAR    ) return "clear";
	if( keyid == KB_RETURN   ) return "return";
	if( keyid == KB_SHIFT    ) return "shift";
	if( keyid == KB_CONTROL  ) return "control";
	if( keyid == KB_MENU     ) return "menu";
	if( keyid == KB_PAUSE    ) return "pause";
	if( keyid == KB_CAPITAL  ) return "capital";
	if( keyid == KB_ESCAPE   ) return "escape";
	if( keyid == KB_SPACE    ) return "space";
	if( keyid == KB_PGUP     ) return "pgup";
	if( keyid == KB_PGDOWN   ) return "pgdown";
	if( keyid == KB_END      ) return "end";
	if( keyid == KB_HOME     ) return "home";
	if( keyid == KB_LEFT     ) return "left";
	if( keyid == KB_UP       ) return "up";
	if( keyid == KB_RIGHT    ) return "right";
	if( keyid == KB_DOWN     ) return "down";
	if( keyid == KB_SNAPSHOT ) return "snapshot";
	if( keyid == KB_INSERT   ) return "insert";
	if( keyid == KB_DELETE   ) return "delete";
	if( keyid == KB_0		 ) return "0";
	if( keyid == KB_1		 ) return "1";
	if( keyid == KB_2		 ) return "2";
	if( keyid == KB_3		 ) return "3";
	if( keyid == KB_4		 ) return "4";
	if( keyid == KB_5		 ) return "5";
	if( keyid == KB_6		 ) return "6";
	if( keyid == KB_7		 ) return "7";
	if( keyid == KB_8		 ) return "8";
	if( keyid == KB_9		 ) return "9";
	if( keyid == KB_A		 ) return "a";
	if( keyid == KB_B		 ) return "b";
	if( keyid == KB_C		 ) return "c";
	if( keyid == KB_D		 ) return "d";
	if( keyid == KB_E		 ) return "e";
	if( keyid == KB_F		 ) return "f";
	if( keyid == KB_G		 ) return "g";
	if( keyid == KB_H		 ) return "h";
	if( keyid == KB_I		 ) return "i";
	if( keyid == KB_J		 ) return "j";
	if( keyid == KB_K		 ) return "k";
	if( keyid == KB_L		 ) return "l";
	if( keyid == KB_M		 ) return "m";
	if( keyid == KB_N		 ) return "n";
	if( keyid == KB_O		 ) return "o";
	if( keyid == KB_P		 ) return "p";
	if( keyid == KB_Q		 ) return "q";
	if( keyid == KB_R		 ) return "r";
	if( keyid == KB_S		 ) return "s";
	if( keyid == KB_T		 ) return "t";
	if( keyid == KB_U		 ) return "u";
	if( keyid == KB_V		 ) return "v";
	if( keyid == KB_W		 ) return "w";
	if( keyid == KB_X		 ) return "x";
	if( keyid == KB_Y		 ) return "y";
	if( keyid == KB_Z		 ) return "z";
	if( keyid == KB_NUMPAD0  ) return "numpad0";
	if( keyid == KB_NUMPAD1  ) return "numpad1";
	if( keyid == KB_NUMPAD2  ) return "numpad2";
	if( keyid == KB_NUMPAD3  ) return "numpad3";
	if( keyid == KB_NUMPAD4  ) return "numpad4";
	if( keyid == KB_NUMPAD5  ) return "numpad5";
	if( keyid == KB_NUMPAD6  ) return "numpad6";
	if( keyid == KB_NUMPAD7  ) return "numpad7";
	if( keyid == KB_NUMPAD8  ) return "numpad8";
	if( keyid == KB_NUMPAD9  ) return "numpad9";
	if( keyid == KB_MULTIPLY ) return "multiply";
	if( keyid == KB_ADD      ) return "add";
	if( keyid == KB_SEPARATOR ) return "separator";
	if( keyid == KB_SUBTRACT ) return "subtract";
	if( keyid == KB_DECIMAL  ) return "decimal";
	if( keyid == KB_DIVIDE   ) return "divide";
	if( keyid == KB_F1       ) return "f1";
	if( keyid == KB_F2       ) return "f2";
	if( keyid == KB_F3       ) return "f3";
	if( keyid == KB_F4       ) return "f4";
	if( keyid == KB_F5       ) return "f5";
	if( keyid == KB_F6       ) return "f6";
	if( keyid == KB_F7       ) return "f7";
	if( keyid == KB_F8       ) return "f8";
	if( keyid == KB_F9       ) return "f9";
	if( keyid == KB_F10      ) return "f10";
	if( keyid == KB_F11      ) return "f11";
	if( keyid == KB_F12      ) return "f12";
	if( keyid == KB_F13      ) return "f13";
	if( keyid == KB_F14      ) return "f14";
	if( keyid == KB_F15      ) return "f15";
	if( keyid == KB_F16      ) return "f16";
	if( keyid == KB_F17      ) return "f17";
	if( keyid == KB_F18      ) return "f18";
	if( keyid == KB_F19      ) return "f19";
	if( keyid == KB_F20      ) return "f20";
	if( keyid == KB_F21      ) return "f21";
	if( keyid == KB_F22      ) return "f22";
	if( keyid == KB_F23      ) return "f23";
	if( keyid == KB_F24      ) return "f24";
	if( keyid == KB_TILDE	 ) return "tilde	";

	return "NULL";
}

/* 
==============================================
uword Input::keyStringToNumber( std::string key )
==============================================
*/
uword Input::keyStringToNumber( std::string key ) {	
	if( key == "back" )		{ return KB_BACK;	}
	if( key == "tab" )		{ return KB_TAB;	}
	if( key == "clear" )	{ return KB_CLEAR ; }
	if( key == "return" )	{ return KB_RETURN; }
	if( key == "shift" )	{ return KB_SHIFT;	}
	if( key == "control" )	{ return KB_CONTROL; }
	if( key == "menu" )		{ return KB_MENU; }
	if( key == "pause" )	{ return KB_PAUSE; }
	if( key == "capital" )	{ return KB_CAPITAL; }
	if( key == "escape" )	{ return KB_ESCAPE; }
	if( key == "space" )	{ return KB_SPACE; }
	if( key == "pgup" )		{ return KB_PGUP; }
	if( key == "pgdown" )	{ return KB_PGDOWN; }
	if( key == "end" )		{ return KB_END; }
	if( key == "home" )		{ return KB_HOME; }
	if( key == "left" )		{ return KB_LEFT; }
	if( key == "up" )		{ return KB_UP; }
	if( key == "right" )	{ return KB_RIGHT; }
	if( key == "down" )		{ return KB_DOWN; }
	if( key == "snapshot " ) { return KB_SNAPSHOT ; }
	if( key == "insert" )	 { return KB_INSERT; }
	if( key == "delete" )	 { return KB_DELETE; }
	if( key == "0" )	 { return KB_0; }
	if( key == "1" )	 { return KB_1; }
	if( key == "2" )	 { return KB_2; }
	if( key == "3" )	 { return KB_3; }
	if( key == "4" )	 { return KB_4; }
	if( key == "5" )	 { return KB_5; }
	if( key == "6" )	 { return KB_6; }
	if( key == "7" )	 { return KB_7; }
	if( key == "8" )	 { return KB_8; }
	if( key == "9" )	 { return KB_9; }
	if( key == "a" )	 { return KB_A; }
	if( key == "b" )	 { return KB_B; }
	if( key == "c" )	 { return KB_C; }
	if( key == "d" )	 { return KB_D; }
	if( key == "e" )	 { return KB_E; }
	if( key == "f" )	 { return KB_F; }
	if( key == "g" )	 { return KB_G; }
	if( key == "h" )	 { return KB_H; }
	if( key == "i" )	 { return KB_I; }
	if( key == "j" )	 { return KB_J; }
	if( key == "k" )	 { return KB_K; }
	if( key == "l" )	 { return KB_L; }
	if( key == "m" )	 { return KB_M; }
	if( key == "n" )	 { return KB_N; }
	if( key == "o" )	 { return KB_O; }
	if( key == "p" )	 { return KB_P; }
	if( key == "q" )	 { return KB_Q; }
	if( key == "r" )	 { return KB_R; }
	if( key == "s" )	 { return KB_S; }
	if( key == "t" )	 { return KB_T; }
	if( key == "u" )	 { return KB_U; }
	if( key == "v" )	 { return KB_V; }
	if( key == "w" )	 { return KB_W; }
	if( key == "x" )	 { return KB_X; }
	if( key == "y" )	 { return KB_Y; }
	if( key == "z" )	 { return KB_Z; }
	if( key == "numpad0" )	 { return KB_NUMPAD0  ; }
	if( key == "numpad1" )	 { return KB_NUMPAD1  ; }
	if( key == "numpad2" )	 { return KB_NUMPAD2  ; }
	if( key == "numpad3" )	 { return KB_NUMPAD3  ; }
	if( key == "numpad4" )	 { return KB_NUMPAD4  ; }
	if( key == "numpad5" )	 { return KB_NUMPAD5  ; }
	if( key == "numpad6" )	 { return KB_NUMPAD6  ; }
	if( key == "numpad7" )	 { return KB_NUMPAD7  ; }
	if( key == "numpad8" )	 { return KB_NUMPAD8  ; }
	if( key == "numpad9" )	 { return KB_NUMPAD9  ; }
	if( key == "multiply" )	 { return KB_MULTIPLY ; }
	if( key == "add" )		 { return KB_ADD; }
	if( key == "separator" )	 { return KB_SEPARATOR; }
	if( key == "subtract " )	 { return KB_SUBTRACT ; }
	if( key == "decimal  " )	 { return KB_DECIMAL  ; }
	if( key == "divide   " )	 { return KB_DIVIDE   ; }
	if( key == "f1" )		 { return KB_F1; }
	if( key == "f2" )		 { return KB_F2; }
	if( key == "f3" )		 { return KB_F3; }
	if( key == "f4" )		 { return KB_F4; }
	if( key == "f5" )		 { return KB_F5; }
	if( key == "f6" )		 { return KB_F6; }
	if( key == "f7" )		 { return KB_F7; }
	if( key == "f8" )		 { return KB_F8; }
	if( key == "f9" )		 { return KB_F9; }
	if( key == "f10" )	 { return KB_F10; }
	if( key == "f11" )	 { return KB_F11; }
	if( key == "f12" )	 { return KB_F12; }
	if( key == "f13" )	 { return KB_F13; }
	if( key == "f14" )	 { return KB_F14; }
	if( key == "f15" )	 { return KB_F15; }
	if( key == "f16" )	 { return KB_F16; }
	if( key == "f17" )	 { return KB_F17; }
	if( key == "f18" )	 { return KB_F18; }
	if( key == "f19" )	 { return KB_F19; }
	if( key == "f20" )	 { return KB_F20; }
	if( key == "f21" )	 { return KB_F21; }
	if( key == "f22" )	 { return KB_F22; }
	if( key == "f23" )	 { return KB_F23; }
	if( key == "f24" )	 { return KB_F24; }
	if( key == "tilde" )	 { return KB_TILDE	;  }

	return NULL;
}