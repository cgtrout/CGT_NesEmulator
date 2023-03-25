#include "precompiled.h"

#include <SDL.h>
#include <algorithm>
#include <thread>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_opengl2.h"
#include "implot/implot.h"

#include "CgtDataStructures.h"

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

	//assign sound device to nesSound
	systemMain->nesMain.nesApu.assignSoundDevice( soundDeviceId );

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
	CgtLib::CircularBuffer<double> frameTimeBuffer( 100 );

	int frameTimeIndex = 0;

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
		//input->clear( );

		//handle sdl input events
		SDL_EventHandler( sdl_event, quit );

		//draw profiler
		if ( drawTimeProfiler ) {
			ImGui::Begin( "Time Profiler", drawTimeProfiler.getPointer( ), 0 );
			ImGui::Text( profilerReport.c_str( ) );

			double max = *std::max_element( frameTimeBuffer.begin( ), frameTimeBuffer.end() ) * 1.3f;
			if ( ImPlot::BeginPlot( "Frame time", ImVec2(-1,0)) ) {
				ImPlot::SetupAxisLimits( ImAxis_Y1, 0, max, ImPlotCond_Always );
				ImPlot::PlotBars( "Frame", frameTimeBuffer.getBufferPtr(), FRAME_TIME_SIZE );
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

		if( capFrameRate.getValue( ) == true ) {
			elapsedTime = std::chrono::steady_clock::now( ) - start_time;

			while( elapsedTime < FRAME_TIME ) {
				//std::this_thread::yield( ); 
				elapsedTime = std::chrono::steady_clock::now( ) - start_time;
			}
		}

		////frame finish
		////
		systemMain->timeProfiler.stopFrame( );
		elapsedTime = std::chrono::steady_clock::now( ) - start_time;
		profilerReport = systemMain->timeProfiler.getSectionReport( );
		
		//handle frame time buffer
		frameTimeBuffer.add( elapsedTime.count( ) );
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
	want.userdata = nullptr;
	want.callback = nullptr;
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

		joy.id = SDL_JoystickInstanceID( joy.handle );

		//replace space with underscore for easier parsing
		std::replace( joy.name.begin( ), joy.name.end( ), ' ', '_' );

		//repalce . with _ as well
		std::replace( joy.name.begin( ), joy.name.end( ), '.', '_' );

		//fill joystick data structure with joystick info
		input->getJoystickMap( )[ joy.id ] = joy;
	}

	//now sort by id so we get proper ordering
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

//last input state - used for button binding
//FIXME decide where to store these, possibly this state should exist in 
//		input class?
std::ostringstream inputLog;
int lastButton = -1;
int lastAxis = -1;
int lastAxisValue = -1;
SDL_Keycode lastKey;
Joystick* lastJoystick;
bool buttonBindMode = false;
bool showButtonBindMenu = true;
int currentController = 0;
int bindIndex = 0;
int lastBoundIndex = 0;
double waitTime = 0;			//if greater than 0 wait
auto startTime = std::chrono::steady_clock::now( );
enum class LastInputType { LASTINPUT_NONE, LASTINPUT_KEYBOARD, LASTINPUT_JOYBUTTON, LASTINPUT_JOYAXIS };
LastInputType lastInputType = LastInputType::LASTINPUT_NONE;

// Constants for axis threshold values
const int AXIS_BIND_THRESHOLD = 20000;

void SDL_EventHandler( SDL_Event& event, bool& quit ) {
	//FIXME - should not at this location
	
	if ( showButtonBindMenu ) {
		ImGui::Begin( "Input", &showButtonBindMenu);

		if( ImGui::Button( "Clear input log" ) ) {
			inputLog.str( "" );
			inputLog.clear( );
		}

		std::string newDeviceName;
		std::string bindName;

		if( buttonBindMode && waitTime <= 0 ) {
			if( ImGui::Button( "Stop button binding" ) ) {
				buttonBindMode = false;
				currentController = 0;
				bindIndex = 0;
			}

			ImGui::Text( "In button bind mode" );
			ImGui::Spacing( );

			auto& controller = input->getControllables( )[ currentController ];
			ImGui::Text( "Current controller is %s", controller->name.c_str( ) );

			auto& button = controller->getButtons( )[ bindIndex ];
			//ImGui::Text( "Button bindName %s", button->bindName.c_str( ) );
			//ImGui::Text( "Button deviceName %s", button->deviceName.c_str( ) );

			switch( lastInputType ) {
			case LastInputType::LASTINPUT_KEYBOARD:
				newDeviceName = "keyboard";
				bindName = input->keyIdToString(lastKey);
				break;
			case LastInputType::LASTINPUT_JOYBUTTON:
				newDeviceName = lastJoystick->name;
				bindName = std::to_string(lastButton);
				break;
			case LastInputType::LASTINPUT_JOYAXIS:
				newDeviceName = lastJoystick->name;
				std::string minmax;

				//find largest axis value and use it?

				if( lastAxisValue < -256 ) {
					minmax = "min";

				}
				else if( lastAxisValue > 256 ) {
					minmax = "max";
				}

				if( !minmax.empty( ) ) {
					bindName = "axis" + std::to_string(lastAxis) + minmax;
				}

				waitTime = 100;

				inputLog << "binding axis: " << std::to_string( lastAxis ) << std::endl;

				break;
			}
			//reset last input type so we don't get repeating key binds on last key
			lastInputType = LastInputType::LASTINPUT_NONE;

			//bindName is name of controller or keyboard bind (button, axis, or keyboard key)
			//TODO - create string

			//prompt user to press key for binding
			ImGui::Text("Binding: nes button %s", button->name.c_str( ));
			ImGui::Text("Press keyboard key or gamepad key to bind this key");

			if( !newDeviceName.empty( ) && !bindName.empty( ) ) {

				//create bind command to send to input system
				input->bindKeyToControl(newDeviceName, bindName, controller->name, button->name);

				//handle indexing to next controller button
				bindIndex++;
				if( bindIndex == controller->getButtons( ).size( ) ) {
					bindIndex = 0;
					currentController++;
					if( currentController == 2 ) buttonBindMode = false;
				}
			}
		}
		else if( buttonBindMode == false ) {
			if( ImGui::Button( "Start button bind" ) ) {
				buttonBindMode = true;
				
				//reset last input so last input doesn't automatically get assigned
				lastInputType = LastInputType::LASTINPUT_NONE;
				currentController = 0;
				bindIndex = 0;
				lastAxisValue = 0;
			}
		}

		const std::string& tmp = inputLog.str( );
		const char* cstr = tmp.c_str( );
		ImGui::Text( "%s", cstr );

		std::chrono::duration<double, std::milli> elapsedTime = std::chrono::steady_clock::now( ) - startTime;

		if( waitTime > 0 ) {
			
			waitTime -= elapsedTime.count();
		}
			
		ImGui::End( );
		startTime = std::chrono::steady_clock::now( );
	}
		
	//take input events and pass them to input system
	//FIXME: move these functions to Input class?
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
					break;
				}

				input->setKeyDown( keydown.sym );
				lastKey = keydown.sym;
				lastInputType = LastInputType::LASTINPUT_KEYBOARD;
				break;
			case SDL_KEYUP:
				SDL_Keysym keyup = event.key.keysym;
				input->setKeyUp( keyup.sym );
				break;
			case SDL_JOYBUTTONDOWN: {
				auto button = event.jbutton.button;
				auto& joystick = input->getJoystickMap( )[ event.jbutton.which ];
				auto deviceName = joystick.name;
				inputLog	<< "buttonDOWN=" << std::to_string( button ) << " " 
							<< "device=" << deviceName
							<< std::endl;
				
				if (joystick.buttonState[ button ] == false) {
					lastButton = button;
					lastJoystick = &joystick;
					joystick.buttonState[ button ] = true;
					lastInputType = LastInputType::LASTINPUT_JOYBUTTON;
				}
				
				break; }
			case SDL_JOYBUTTONUP: {
				auto button = event.jbutton.button;
				auto& joystick = input->getJoystickMap( )[ event.jbutton.which ];
				auto deviceName = joystick.name;
				inputLog << "buttonUP=" << std::to_string( button ) << " "
					<< "device=" << deviceName
					<< std::endl;

				joystick.buttonState[ button ] = false;

				break; }
			case SDL_JOYAXISMOTION:
			{
				Joystick& joystick = input->getJoystickMap()[event.jbutton.which];

				inputLog << "startJoystick=" << std::to_string((int)joystick.handle);
				inputLog << " axis =" << std::to_string(event.jaxis.axis) << " "
						<< " value=" << std::to_string(event.jaxis.value) << std::endl;

				joystick.axisState[event.jaxis.axis] = event.jaxis.value;

				if (buttonBindMode && abs(event.jaxis.value) > AXIS_BIND_THRESHOLD) {
					if ((lastAxisValue > AXIS_BIND_THRESHOLD && event.jaxis.value < AXIS_BIND_THRESHOLD)
						|| (lastAxisValue < AXIS_BIND_THRESHOLD && event.jaxis.value > AXIS_BIND_THRESHOLD)
						|| lastAxisValue == 0)
					{
						lastJoystick = &joystick;
						lastAxis = event.jaxis.axis;
						lastAxisValue = event.jaxis.value;

						inputLog << "Setting last: " << std::to_string((int)joystick.handle);
						inputLog << " axis: " << std::to_string(lastAxis);
						inputLog << std::endl;

						lastInputType = LastInputType::LASTINPUT_JOYAXIS;
					}
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
}

void switchFullscreen( ) {
	//switch to full screen sdl
	if( isWindowMode ) {
		if( SDL_SetWindowFullscreen( window, SDL_WINDOW_FULLSCREEN_DESKTOP ) < 0 ) {
			const char* error = SDL_GetError( );
			SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Full screen Switch Error", error, window );
			SDL_Quit( );
		}
	}
	else {
		if( SDL_SetWindowFullscreen( window, 0 ) < 0 ) {
			const char* error = SDL_GetError( );
			SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Window Mode Switch Error", error, window );
			SDL_Quit( );
		}
	}
	VsyncHandler( window );
	isWindowMode = !isWindowMode;
}

void initializeEmulator( ) {
	systemMain = FrontEnd::SystemMain::getInstance( );
	consoleSystem = &systemMain->consoleSystem;
	systemMain->initialize( );
	systemMain->renderer.setRes( SCREEN_WIDTH, SCREEN_HEIGHT );

	systemMain->start( );

	//set frame rate counter pos based on screen size

	input = FrontEnd::InputSystem::Input::getInstance( );

	const double FRAME_TIME = 1.0f / 60.0f;

	bool freshFrame = true;
}