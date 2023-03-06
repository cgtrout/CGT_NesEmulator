#include "precompiled.h"

#include <SDL.h>

constexpr auto SCREEN_WIDTH = 800;
constexpr auto SCREEN_HEIGHT = 600;

Console::ConsoleVariable< bool > capFrameRate(
	/*start val*/	false,
	/*name*/		"capFrameRate",
	/*description*/	"Caps frame rate to 60hz if set to true",
	/*save?*/		SAVE_TO_FILE );

void initializeEmulator( );

int main( int argc, char* args[] )
{
	SDL_Window* window = nullptr;
	SDL_Surface* screenSurface = nullptr;

	if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
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

	SDL_GLContext context = SDL_GL_CreateContext( window );

	SDL_Event e;
	bool quit = false;

	initializeEmulator( );
	SDL_StopTextInput( );

	//main loop
	while ( quit == false ) {
		while ( SDL_PollEvent( &e ) ) {
			switch ( e.type ) {
				case SDL_QUIT:
					quit = true;
					break;
				case SDL_KEYDOWN:
					SDL_Keysym keydown = e.key.keysym;
					input->setKeyDown( keydown.sym );
					break;
				case SDL_KEYUP:
					SDL_Keysym keyup = e.key.keysym;
					input->setKeyUp( keyup.sym );
					break;
				case SDL_TEXTINPUT:
					input->addTextInput( e.text.text );
					break;
				default:
					break;
			}	
		}
		//frame initialization
		auto start_time = std::chrono::steady_clock::now( );
		std::chrono::duration<double> elapsedTime = std::chrono::steady_clock::now( ) - start_time;
		systemMain->timeProfiler.startFrame( );
		systemMain->runFrame( );
		SDL_GL_SwapWindow( window );
		
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

	systemMain->consoleSystem.variables.addBoolVariable( &capFrameRate );
	bool freshFrame = true;
}