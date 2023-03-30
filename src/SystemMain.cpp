// SystemMain.cpp: implementation of the SystemMain class.
//
//////////////////////////////////////////////////////////////////////
#include "precompiled.h"

#include "SystemMain.h"

#include <cmath>

#ifndef LIGHT_BUILD
  #include "NesDebugger.h"
#endif  #end

using namespace GUISystem;
using namespace FrontEnd;
using namespace Render;
using namespace NesEmulator;

#include <SDL_scancode.h>
#include <filesystem>

#include "Console.h"
#include "NesMain.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl2.h"

#include "SystemMain.h"
using namespace Console;
using namespace InputSystem;

/*
==============================================
SystemMain::SystemMain()
==============================================
*/
SystemMain::SystemMain() :
	nesMain( this ),
	gui( &input ),
	renderer( this ),
	guiConsole{ &input }
{
	//initialize systems
	consoleSystem.initialize();

	_log = CLog::getInstance();
	_log->Init();
	
	input.init();
	input.clear();	

	//make sure that nesFile is loaded into memory before we load nes file
	nesMain.nesFile.initialize();

	wantsQuit = false;
	
	input.setUseDelay( true );
}

void SystemMain::initialize( ) {
	gui.initialize( );
	nesMain.nesCpu.initialize( );
	nesMain.nesMemory.initialize( );
	nesMain.nesPpu.initialize( );
	nesMain.nesPpu.scanlineDrawer.initialize( );
	nesMain.nesPpu.initialize( );

#ifndef LIGHT_BUILD
	nesMain.nesDebugger.initialize( );
#endif

}


void SystemMain::loadNesFile( std::string_view fileName ) {
	//TODO move to seperate function in nesmain
	nesMain.nesApu.setUninitialized( );
	nesMain.setState( WaitingForFile );
	nesMain.nesFile.loadFile( fileName );
	auto message = std::string( "NES file load successful:" ) + std::string(fileName);
	consoleSystem.printMessage( message.c_str() );

	nesMain.nesPpu.reset( );
	nesMain.reset( );
	nesMain.nesCpu.reset( );
	//systemMain->nesMain.nesCpu.setOnStatus( true );
	nesMain.nesApu.setInitialized( );

	nesMain.setState( Emulating );
	nesMain.enableStepDebugging( "START" );
}

/*
==============================================
SystemMain::~SystemMain()
==============================================
*/
SystemMain::~SystemMain() {
	input.writeBindsToFile( "binds.cfg" );
}

/*
==============================================
SystemMain::start()
==============================================
*/
void SystemMain::start() {
	//if binds file not present load from default binds
	if ( std::filesystem::exists( "binds.cfg" ) ) {
		consoleSystem.loadCommandFile( "binds.cfg" );
	} else {
		if( std::filesystem::exists( "default-binds.cfg" ) ) {
			consoleSystem.loadCommandFile( "default-binds.cfg" );
		}
	}
	if( std::filesystem::exists( "autoexec.cfg" ) ) {
		consoleSystem.loadCommandFile( "autoexec.cfg" );
	}

	renderer.initialize();

	//add gui console
	gui.addElement( &guiConsole );
	gui.setUsingMouse( true );
	guiConsole.setOpen( false );
}

/*
==============================================
SystemMain::runFrame()
==============================================
*/
void SystemMain::runFrame() {
	renderer.initFrame();

	//update all controls linked to input
	input.updateControllables();

	//tilde key
	if( input.isKeyDownUnset( SDLK_BACKQUOTE ) ) {
		guiConsole.setOpen( !guiConsole.isOpen() );
		
		if ( guiConsole.isOpen( ) ) {
			guiConsole.editLine.setAsActiveElement( );
		}
 else {
	 input.setState( Input::InputSystemStates::NORMAL_MODE );
	 guiConsole.editLine.unactivateElement( );
		}
	}

#ifndef LIGHT_BUILD
	//see if escape key is down for quitting
	if( input.isKeyDown( SDLK_BACKSPACE ) ) {
		if( !nesMain.nesDebugger.inSingleStepMode( ) ) {
			nesMain.nesDebugger.turnOffSingleStepMode( );
		}
	}

	//F5 controls single stepping with the debugger
	if( input.isKeyDownUnset( SDLK_F5 ) ) {
		if( !nesMain.nesDebugger.inSingleStepMode( ) ) {
			if( nesMain.getState( ) == Emulating ) {
				nesMain.nesDebugger.setToSingleStepMode( nesMain.nesCpu.getPC( ), "" );
			}
		}
		else {
			nesMain.nesDebugger.turnOffSingleStepMode( );
		}
	}
	if( input.isKeyDownUnset( SDLK_F9 ) ) {
		nesMain.nesDebugger.addBreakPoint( nesMain.nesDebugger.getSelectedAddress( ) );
	}
	if( input.isKeyDownUnset( SDLK_F6 ) ) {
		if( nesMain.nesDebugger.inSingleStepMode( ) ) {
			nesMain.nesDebugger.singleStepRequest( );
		}
	}
	if( input.isKeyDown( SDLK_RETURN ) ) {
		if( nesMain.nesDebugger.isOpen( ) ) {
			nesMain.nesDebugger.onEnter( );
		}
	}
#endif 
	timeProfiler.startSection( "Gui" );
	gui.runFrame( );
	nesMain.runFrame( );
	graphicUpdate( );

	timeProfiler.stopActive( );

	input.clear( );
}

/*
==============================================
SystemMain::graphicUpdate()
==============================================
*/
void SystemMain::graphicUpdate( ) {
	timeProfiler.startSection( "APU Visualization" );
	if( nesMain.nesApu.isInitialized( ) ) {
		nesMain.nesApu.getNesSoundBuffer( )->renderImGui( );
	}

	timeProfiler.startSection( "RenFrame" );
	renderer.renderFrame( );

	timeProfiler.stopActive( );
}

/*
==============================================
SystemMain::loadNextTest()
==============================================
*/
void SystemMain::loadNextTest( ) {
	if( testingSystem.files.empty( ) == true ) {
		return;
	}

	if( testingSystem.iter != testingSystem.files.end( ) ) {
		//get name of next file
		auto nextFileName = *( testingSystem.iter );
		auto withDir = testingSystem.directory + nextFileName;
		loadNesFile( withDir );
		testingSystem.iter++;
	}
	if( testingSystem.iter == testingSystem.files.end( ) ) {
		consoleSystem.printMessage( "End of tests" );
	}
}

/*
==============================================
TestingSystem::buildDirVector()
==============================================
*/
void SystemMain::TestingSystem::buildDirVector( std::string_view dirName ) {
	files.clear( );
	directory = dirName;
	for( const auto& entry : std::filesystem::directory_iterator( dirName ) ) {
		auto filepath = entry.path( ).filename( ).string( );
		//confirm last three chars are nes
		if( filepath.substr( filepath.length( ) - 3 , filepath.length( ) ) != "nes") {
			continue;
		}

		//remove .nes
		filepath = filepath.substr( 0,  filepath.length( ) - 4 );
		files.push_back( filepath );
	}
	std::sort( files.begin( ), files.end( ) );

	iter = files.begin( );
}

