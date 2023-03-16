#include "precompiled.h"

#include <SDL.h>
#include <algorithm>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_opengl2.h"
#include "implot/implot.h"

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

Console::ConsoleVariable< bool > drawTimeProfiler(
	/*start val*/	true,
	/*name*/		"drawTimeProfiler",
	/*description*/	"Show time profiler window",
	/*save?*/		SAVE_TO_FILE );

//variables
SDL_Window* window = nullptr;
bool isWindowMode = true;


//function declarations
void initializeEmulator( );
void SDL_EventHandler( SDL_Event& event, bool& quit );
void switchFullscreen( );
bool VsyncHandler( SDL_Window* window );
SDL_AudioDeviceID initializeSound( SDL_Window* window, SDL_AudioSpec* outAudioSpec );
void initializeVideo( SDL_Window*& window );
void initializeJoysticks( );
void audioCallbackFunction( void* unused, Uint8* stream, int len );

void nesAudio( Uint8* stream, int len );

void sinewaveTest( Uint8* stream, int len );

int main( int argc, char* args[] )
{
	SDL_Surface* screenSurface = nullptr;
	
	//audio specification given back by audio system
	SDL_AudioSpec audioSpecification;

	initializeVideo( window );
	SDL_AudioDeviceID soundDeviceId = initializeSound( window, &audioSpecification );

	//initialize emulation systems
	initializeEmulator( );

	//initialize SDL joysticks
	initializeJoysticks( );

	//start sound
	SDL_PauseAudioDevice( soundDeviceId, 0 );

	//insert console variables for capping frame rate and setting vsync
	consoleSystem->variables.addBoolVariable( &capFrameRate );
	consoleSystem->variables.addBoolVariable( &vsync );
	consoleSystem->variables.addBoolVariable( &drawTimeProfiler );
	
	//set vsync based on console variable setting
	VsyncHandler( window );

	//SDL loop variables
	SDL_Event sdl_event;
	bool quit = false;
	bool vsyncSetting = vsync.getValue( );
	std::string profilerReport{};

	static const int FRAME_TIME_SIZE = 100;
	std::array<double, 1000> frameTime{};
	int frameTimeIndex = 0;
	frameTime.fill( 0.0f );

	//ensure SDL isn't initialized to take text input rather than raw presses
	SDL_StopTextInput( );

	//main sdl loop
	while ( quit == false ) {
		//check to see if vsync needs to be set
		if ( vsyncSetting != vsync.getValue( ) ) {
			vsyncSetting = VsyncHandler( window );
		}
		
		////frame initialization
		////
		auto start_time = std::chrono::steady_clock::now( );
		std::chrono::duration<double> elapsedTime = std::chrono::steady_clock::now( ) - start_time;
		systemMain->timeProfiler.startFrame( );

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL2_NewFrame( );
		ImGui_ImplSDL2_NewFrame( );
		ImGui::NewFrame( );
		
		//reset input state
		input->clear( );

		//handle sdl input events
		SDL_EventHandler( sdl_event, quit );

		//draw profiler
		if ( drawTimeProfiler ) {
			ImGui::Begin( "Time Profiler", drawTimeProfiler.getPointer( ), 0 );
			ImGui::Text( profilerReport.c_str( ) );

			double max = *std::max_element( frameTime.begin( ), frameTime.end() ) * 1.3f;
			if ( ImPlot::BeginPlot( "Frame time", ImVec2(-1,0)) ) {
				ImPlot::SetupAxisLimits( ImAxis_Y1, 0, max, ImPlotCond_Always );
				ImPlot::PlotBars( "Frame", frameTime.data(), FRAME_TIME_SIZE );
				ImPlot::EndPlot( );
			}
			ImGui::End( );
		}
		
		////run frame
		////
		systemMain->runFrame( );

		//do imgui handling here

		//static bool showDemo = false;
		//ImGui::ShowDemoWindow( &showDemo );

		SDL_GL_SwapWindow( window );

		if ( capFrameRate.getValue() == true ) {
			while ( elapsedTime.count( ) < FRAME_TIME ) {
				elapsedTime = std::chrono::steady_clock::now( ) - start_time;
			}
		}

		////frame finish
		////
		systemMain->timeProfiler.stopFrame( );
		elapsedTime = std::chrono::steady_clock::now( ) - start_time;
		profilerReport = systemMain->timeProfiler.getSectionReport( );
		
		//handle frame time buffer
		frameTime[ frameTimeIndex++ ] = elapsedTime.count( );
		if ( frameTimeIndex == FRAME_TIME_SIZE ) {
			frameTimeIndex = 0;
		}

		systemMain->fpsTimer.updateTimer( elapsedTime.count( ) );		
	}

	ImGui::DestroyContext( );
	ImPlot::DestroyContext( );
	SDL_DestroyWindow( window );
	SDL_Quit( );

	return 0;
}

void initializeVideo( SDL_Window*& window )
{
	if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK ) < 0 ) {
		const char* error = SDL_GetError( );
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Initialization Error", error, window );
		SDL_Quit( );
	}

	window = SDL_CreateWindow( "CGT Nes Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE );

	if ( window == nullptr ) {
		const char* error = SDL_GetError( );
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Initialization Error", error, window );
		SDL_Quit( );
	}

	
	//initialize OpenGL
	SDL_GLContext context = SDL_GL_CreateContext( window );

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION( );
	ImGui::CreateContext( );
	ImPlot::CreateContext( );

	ImGuiIO& io = ImGui::GetIO( ); ( void )io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	ImGui::StyleColorsDark( );
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL( window, context );
	ImGui_ImplOpenGL2_Init( );

	isWindowMode = true;
}

//outAudioSpec is the returned audio spec
//returns deviceId that audio was assigned to
SDL_AudioDeviceID initializeSound( SDL_Window* window, SDL_AudioSpec* outAudioSpec )
{
	//initialize sound
	SDL_AudioSpec want;
	want.freq = AUDIO_SAMPLE_RATE;
	want.format = AUDIO_S16;
	want.samples = AUDIO_SAMPLES;
	want.callback = audioCallbackFunction;
	want.userdata = nullptr;
	want.channels = 1;
	auto deviceId = SDL_OpenAudioDevice( nullptr, 0, &want, outAudioSpec, 0 );
	if ( deviceId < 0 ) {
		const char* error = SDL_GetError( );
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Sound init Error", error, window );
		SDL_Quit( );
	}

	//TODO validate that want and have are the same?

	return deviceId;
}

void initializeJoysticks( )
{
	//init joysticks
	auto numJoysticks = SDL_NumJoysticks( );
	for ( int i = 0; i < numJoysticks; i++ ) {
		Joystick joy{ };
		joy.handle = SDL_JoystickOpen( i );
		joy.name = SDL_JoystickName( joy.handle );

		//replace space with underscore for easier parsing
		std::replace( joy.name.begin( ), joy.name.end( ), ' ', '_' );

		input->getJoysticks( ).push_back( joy );
	}
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
std::ostringstream inputLog;
void SDL_EventHandler( SDL_Event& event, bool& quit ) {
	ImGui::Begin( "Input log" );
	
	
	//take input events and pass them to input system
	while ( SDL_PollEvent( &event ) ) {
		ImGui_ImplSDL2_ProcessEvent( &event );
		switch ( event.type ) {
			case SDL_QUIT:
				quit = true;
				break;
			case  SDL_WINDOWEVENT:
				//give width height to renderer object
				if( event.window.event == SDL_WINDOWEVENT_RESIZED )	{
					int w = 0;	int h = 0;
					SDL_GetWindowSize( window, &w, &h );
					systemMain->renderer.setXYRes( w, h ); 
					systemMain->renderer.resizeInitialize( );
				}
				break;
			case SDL_KEYDOWN:
				SDL_Keysym keydown = event.key.keysym;

				//detect alt+enter for full screen switch
				if ( ( keydown.sym == SDLK_RETURN ) && ( keydown.mod & KMOD_ALT ) ) {
					switchFullscreen( );
				}

				input->setKeyDown( keydown.sym );
				break;
			case SDL_KEYUP:
				SDL_Keysym keyup = event.key.keysym;
				input->setKeyUp( keyup.sym );
				break;
			case SDL_JOYBUTTONDOWN: {
				auto button = event.jbutton.button;
				auto& joystick = input->getJoysticks( )[ event.jbutton.which ];
				auto deviceName = joystick.name;
				inputLog	<< "button=" << std::to_string( button ) << " " 
							<< "device=" << deviceName
							<< std::endl;

				joystick.buttonState[ button ] = true;
				
				break; }
			case SDL_JOYAXISMOTION: {
				if ( ( event.jaxis.value < -3200 ) || ( event.jaxis.value > 3200 ) ) {
					inputLog	<< "axis =" << std::to_string(event.jaxis.axis) << " "
								<< "value=" << std::to_string( event.jaxis.value ) << std::endl;
				}
				break; 
			}
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

	const std::string& tmp = inputLog.str( );
	const char* cstr = tmp.c_str( );
	ImGui::Text( "%s", cstr );
	ImGui::End( );
}

void switchFullscreen( ) {
	//switch to full screen sdl
	if ( isWindowMode ) {
		if ( SDL_SetWindowFullscreen( window, SDL_WINDOW_FULLSCREEN_DESKTOP ) < 0 ) {
			const char* error = SDL_GetError( );
			SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Full screen Switch Error", error, window );
			SDL_Quit( );
		}
	} else {
		if ( SDL_SetWindowFullscreen( window, 0 ) < 0 ) {
			const char* error = SDL_GetError( );
			SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Window Mode Switch Error", error, window );
			SDL_Quit( );
		}
	}

	isWindowMode = !isWindowMode;
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

//sine wave audio functions - to generate test 'tone'
double timeToRadian( double intime, double hertz ) {
	return 2.0f * intime * hertz * M_PI;
}
double waveFormSine( double intime, double hertz ) {
	return sin( timeToRadian( intime, hertz ) );
}

//get seconds per sample
double getTimePerSample( int samplerate ) {
	return 1.0f / ( double )samplerate;
}

//convert a double float to 16bit sample
//fixme: convert to signed int
Uint16 doubleTo16bit( double f ) {
	return (Uint16)(f * 0x7FFF);
}

//audio callback function
void audioCallbackFunction( void* unused, Uint8* stream, int len ) {
	//sinewaveTest( stream, len );

	nesAudio( stream, len );
}

void nesAudio( Uint8* stream, int len )
{
	//we are using 16 bit samples rather than eight, so cast 8bit pointer to a 16 bit pointer
	//fixme: convert to signed int
	Sint16* pointer16 = ( Sint16* )&stream[ 0 ];

	auto* apu = &systemMain->nesMain.nesApu;

	if ( apu->isInitialized( ) ) {
		auto* buffer = apu->getNesSoundBuffer( );
		buffer->fillExternalBuffer( pointer16, len / 2 );
	}
	
	//TODO - need to verify that entire buffer is filled (verify this)
}

//run stream through here to generate 440hz test tone (sine wave)
void sinewaveTest( Uint8* stream, int len ) {
	double timePerSample = getTimePerSample( AUDIO_SAMPLE_RATE );
	static double accumulatedTime = 0.0f;
	float hzTone = 440.f;

	//we are using 16 bit samples rather than eight, so cast 8bit pointer to a 16 bit pointer
	//fixme: convert to signed int
	Uint16* pointer16 = ( Uint16* )&stream[ 0 ];

	for ( int i = 0; i < len / 2; i++ ) {
		pointer16[ i ] = doubleTo16bit( waveFormSine( accumulatedTime + ( double )i * timePerSample, hzTone ) );
	}

	//ensure accumulated time doesn't grow forever
	//it will always wrap every (1/hz)
	accumulatedTime += ( len / 2.0f ) * timePerSample;
	while ( accumulatedTime >( 1 / hzTone ) ) {
		accumulatedTime -= ( 1 / hzTone );
	}
}
