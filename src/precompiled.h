#include "SystemMain.h"
#include "CgtException.h"
#include "CGTSingleton.h"
#include "StringToNumber.h"
#include "Console.h"
#include "ConsoleCommands.h"

using namespace CGTSingleton;

#include "Timer.h"
#include "Log.h"

//turn off unsafe string function warnings
//TODO change these someday...
#if _MSC_VER > 1000
#pragma warning( disable : 4996 )
#endif

#ifdef WIN32 
#define SYSTEM_PLATFORM = "Win32"
#endif


//define this to build a light weight version of the emulator
//no extra debugging systems
//#define LIGHT_BUILD

extern FrontEnd::SystemMain	*systemMain;
extern FrontEnd::InputSystem::Input *input;

extern FrontEnd::CLog *_log;
extern FrontEnd::Timer *timer;

extern Console::ConsoleSystem *consoleSystem;

//extract bit "bit" from value
//read as " extract bit 'bit' from number 'value' "
#define BIT( bit, value ) ( ( value & ( 1 << bit ) ) >> bit )

//flip bit "bit" in value
#define FLIP_BIT( bit, value ) (  (( !( (value & ( 1 << bit )) >> bit ) << bit ) + ( value & ~( 1 << bit ) )))

//#define CYCLES_PER_FRAME 89492
#define CYCLES_PER_FRAME 89342

//must be set to this to pass nmi clearing test 
//it seems higher than it should have to be though...
const int vblankOffTime = 6810;//6821;//7161;


template< class T >
T FORCE_LOW( const T val, const T lowestAllowed ) {
	if( val < lowestAllowed ) {
		return lowestAllowed;
	} else {
		return val;
	}
}

template< class T >
T FORCE_HIGH( const T val, const T highAllowed ) {
	if( val > highAllowed ) {
		return highAllowed;
	} else {
		return val;
	}
}

#include "typedefs.h"

