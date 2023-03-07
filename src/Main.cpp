#include "precompiled.h"

#include <SDL.h>

constexpr auto SCREEN_WIDTH = 800;
constexpr auto SCREEN_HEIGHT = 600;
constexpr double FRAME_TIME = 1.0f / 60.0f;

Console::ConsoleVariable< bool > capFrameRate(
	/*start val*/	false,
	/*name*/		"capFrameRate",
	/*description*/	"Caps frame rate to 60hz if set to true",
	/*save?*/		SAVE_TO_FILE );

Console::ConsoleVariable< bool > vsync (
	/*start val*/	true,
	/*name*/		"vsync",
	/*description*/	"Enable vsync",
	/*save?*/		SAVE_TO_FILE );

//function declarations
void initializeEmulator( );
void SDL_EventHandler( SDL_Event& event, bool& quit );
bool VsyncHandler( SDL_Window* window );
void audioCallbackFunction( void* unused, Uint8* stream, int len );

int main( int argc, char* args[] )
{
	SDL_Window* window = nullptr;
	SDL_Surface* screenSurface = nullptr;

	if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 ) {
		const char *error = SDL_GetError( );
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Initialization Error", error, window );
		SDL_Quit( );
	}

	window = SDL_CreateWindow( "CGT Nes Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL );

	if ( window == nullptr ) {
		const char *error = SDL_GetError( );
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Initialization Error", error, window );
		SDL_Quit( );
	}

	//initialize OpenGL
	SDL_GLContext context = SDL_GL_CreateContext( window );

	//initialize sound
	SDL_AudioSpec as;
	as.freq = 44100;
	as.format = AUDIO_S16;
	as.samples = 4096;
	as.callback = audioCallbackFunction;
	as.userdata = nullptr;
	as.channels = 1;
	if ( SDL_OpenAudio( &as, nullptr ) < 0 ) {
		const char* error = SDL_GetError( );
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Sound init Error", error, window );
		SDL_Quit( );
	}
	
	//initialize emulation systems
	initializeEmulator( );

	//insert console variables
	consoleSystem->variables.addBoolVariable( &capFrameRate );
	consoleSystem->variables.addBoolVariable( &vsync );
	
	//set vsync based on console variable setting
	VsyncHandler( window );

	//SDL loop variables
	SDL_Event sdl_event;
	bool quit = false;
	bool vsyncSetting = vsync.getValue( );

	//ensure SDL isn't initialized to take text input rather than raw presses
	SDL_StopTextInput( );

	//main sdl loop
	while ( quit == false ) {
		//check to see if vsync needs to be set
		if ( vsyncSetting != vsync.getValue( ) ) {
			vsyncSetting = VsyncHandler( window );
		}
		
		//frame initialization
		auto start_time = std::chrono::steady_clock::now( );
		std::chrono::duration<double> elapsedTime = std::chrono::steady_clock::now( ) - start_time;
		systemMain->timeProfiler.startFrame( );
		
		//reset input state
		input->clear( );

		//handle sdl input events
		SDL_EventHandler( sdl_event, quit );
		
		//run frame
		systemMain->runFrame( );
		SDL_GL_SwapWindow( window );

		if ( capFrameRate.getValue() == true ) {
			while ( elapsedTime.count( ) < FRAME_TIME ) {
				elapsedTime = std::chrono::steady_clock::now( ) - start_time;
			}
		}

		//frame finish
		systemMain->timeProfiler.stopFrame( );
		elapsedTime = std::chrono::steady_clock::now( ) - start_time;
		systemMain->guiTimeProfiler.setReportString( systemMain->timeProfiler.getSectionReport( ) );
		systemMain->fpsTimer.updateTimer( elapsedTime.count( ) );		
	}

	SDL_DestroyWindow( window );
	SDL_Quit( );

	return 0;
}

bool VsyncHandler( SDL_Window* window ) {
	int vsyncError = -2;
	if ( vsync.getValue( ) == true ) {
		vsyncError = SDL_GL_SetSwapInterval( 1 );
		_log->Write( "Vsync Handler: setting to ON" );
		return true;
	} else {
		vsyncError = SDL_GL_SetSwapInterval( 0 );
		_log->Write( "Vsync Handler: setting to OFF" );
		return false;
	}
	//show error if vsync was not successfull
	if ( vsyncError == -1 ) {
		const char* error = SDL_GetError( );
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "VSYNC Error", error, window );
	}
}

void SDL_EventHandler( SDL_Event& event, bool& quit ) {
	//take input events and pass them to input system
	while ( SDL_PollEvent( &event ) ) {
		switch ( event.type ) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
				SDL_Keysym keydown = event.key.keysym;
				input->setKeyDown( keydown.sym );
				break;
			case SDL_KEYUP:
				SDL_Keysym keyup = event.key.keysym;
				input->setKeyUp( keyup.sym );
				break;
			case SDL_TEXTINPUT:
				input->addTextInput( event.text.text );
				break;
			case SDL_MOUSEBUTTONDOWN:
				switch ( event.button.button ) {
					case SDL_BUTTON_LEFT:
						input->setMouseLeftDown( true );
						break;
					case SDL_BUTTON_RIGHT:
						input->setMouseRightDown( true );
						break;
				}
				break;
			case SDL_MOUSEBUTTONUP:
				switch ( event.button.button ) {
					case SDL_BUTTON_LEFT:
						input->setMouseLeftUp( true );
						break;
					case SDL_BUTTON_RIGHT:
						input->setMouseRightUp( true );
						break;
				}
				break;
			case SDL_MOUSEMOTION:
				input->setMouseX( event.motion.x );
				input->setMouseY( event.motion.y );
				break;
			default:
				break;
		}
	}
}

void initializeEmulator( ) {
	systemMain = FrontEnd::SystemMain::getInstance( );
	consoleSystem = &systemMain->consoleSystem;
	systemMain->initialize( );
	systemMain->renderer.setRes( SCREEN_WIDTH, SCREEN_HEIGHT );

	//assign soundsystem
	// TODO
	//systemMain->soundSystem = &winSoundSystem;

	systemMain->start( );

	//set frame rate counter pos based on screen size
	systemMain->frameCounter.setX( SCREEN_WIDTH - 87 );
	systemMain->frameCounter.setY( SCREEN_HEIGHT - 20 );

	input = FrontEnd::InputSystem::Input::getInstance( );

	systemMain->fpsTimer.clearTimer( );

	const double FRAME_TIME = 1.0f / 60.0f;

	bool freshFrame = true;
}

//audio callback function
void audioCallbackFunction( void* unused, Uint8* stream, int len ) {

}
