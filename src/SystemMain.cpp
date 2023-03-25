// SystemMain.cpp: implementation of the SystemMain class.
//
//////////////////////////////////////////////////////////////////////
#include "precompiled.h"

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

#include "ImageTools.H"

#ifndef LIGHT_BUILD
  extern NesDebugger *nesDebugger;
#endif
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl2.h"

using namespace Console;
using namespace InputSystem;

/*
==============================================
SystemMain::SystemMain()
==============================================
*/
SystemMain::SystemMain() {
	//initialize systems
	consoleSystem.initialize();

	_log = CLog::getInstance();
	_log->Init();
	
	input = Input::getInstance();
	input->init();
	input->clear();	
	input->clearInputState();

	//make sure that nesFile is loaded into memory before we load nes file
	nesMain.nesFile.initialize();

	wantsQuit = false;
	
	input->setUseDelay( true );
}

void SystemMain::initialize( ) {
	gui.initialize( );
	nesMain.nesCpu.initialize( );
	nesMain.nesMemory.initialize( );
	nesMain.nesPpu.initialize( );
	nesMain.nesPpu.scanlineDrawer.initialize( );
	nesMain.nesPpu.initialize( );

#ifndef LIGHT_BUILD
	nesDebugger->initialize( );
#endif

}

/*
==============================================
SystemMain::~SystemMain()
==============================================
*/
SystemMain::~SystemMain() {
	input->writeBindsToFile( "binds.cfg" );
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
	input->updateControllables();

	//tilde key
	if( input->isKeyDownUnset( SDLK_BACKQUOTE ) ) {
		guiConsole.setOpen( !guiConsole.isOpen() );
		
		if ( guiConsole.isOpen( ) ) {
			guiConsole.editLine.setAsActiveElement( );
		} else {
			input->setState( Input::InputSystemStates::NORMAL_MODE );
			guiConsole.editLine.unactivateElement( );
		}
	}
	
#ifndef LIGHT_BUILD
	//see if escape key is down for quitting
	if( input->isKeyDown( SDLK_BACKSPACE ) ) {
		if( !nesMain.nesDebugger.inSingleStepMode() ) {
			nesMain.nesDebugger.turnOffSingleStepMode();
		}
	}
	
	//F5 controls single stepping with the debugger
	if( input->isKeyDownUnset( SDLK_F5 ) ) {
		if( !nesMain.nesDebugger.inSingleStepMode() ) {
			if( nesMain.getState() == Emulating ) {
				nesMain.nesDebugger.setToSingleStepMode( nesMain.nesCpu.getPC() );
			}
		}
		else {
			nesMain.nesDebugger.turnOffSingleStepMode();
		}
	}
	if( input->isKeyDownUnset( SDLK_F9 ) ) {
		nesMain.nesDebugger.addBreakPoint( nesMain.nesDebugger.getSelectedAddress() );
	}
	if( input->isKeyDownUnset( SDLK_F6 ) ) {
		if( nesMain.nesDebugger.inSingleStepMode() ) {
			nesMain.nesDebugger.singleStepRequest();
		}
	}
	if( input->isKeyDown( SDLK_RETURN ) ) {
		if( nesMain.nesDebugger.isOpen() ) {
			nesMain.nesDebugger.onEnter();
		}
	}	
#endif 
	
	gui.runFrame();
	nesMain.runFrame();
	
	
	graphicUpdate();

	timeProfiler.stopActive( );
}

/*
==============================================
SystemMain::graphicUpdate()
==============================================
*/
void SystemMain::graphicUpdate() {
	timeProfiler.startSection( "APU Visualization" );
	if( nesMain.nesApu.isInitialized( ) ) {
		nesMain.nesApu.getNesSoundBuffer( )->renderImGui( );
	}

	timeProfiler.startSection( "RenFrame" );
	renderer.renderFrame();
	
	timeProfiler.stopActive( );
}

