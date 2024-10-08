#pragma once

#include "Log.h"
#include "Console.h"

#include <chrono>

//define this to build a light weight version of the emulator
//no extra debugging systems
//#define LIGHT_BUILD

extern FrontEnd::CLog *_log;
extern Console::ConsoleSystem *consoleSystem;

//extract bit "bit" from value
//read as " extract bit 'bit' from number 'value' "
#define BIT( bit, value ) ( ( value & ( 1 << bit ) ) >> bit )

//flip bit "bit" in value
#define FLIP_BIT( bit, value ) (  (( !( (value & ( 1 << bit )) >> bit ) << bit ) + ( value & ~( 1 << bit ) )))

//#define CYCLES_PER_FRAME 89492
#define CYCLES_PER_FRAME 89342

constexpr auto SCREEN_WIDTH = 1280;
constexpr auto SCREEN_HEIGHT = 764;
constexpr auto TARGET_FPS = 60.0f;
constexpr auto FRAME_TIME = std::chrono::duration<double>( 1.0 / TARGET_FPS );
constexpr auto AUDIO_SAMPLE_RATE = 44100;
constexpr auto AUDIO_SAMPLES = 4096;

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

