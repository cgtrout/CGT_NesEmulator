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
#include "Graph.h"

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
	try {
		//if binds file not present load from default binds
		if ( std::filesystem::exists( "binds.cfg" ) ) {
			consoleSystem.loadCommandFile( "binds.cfg" );
		} else {
			consoleSystem.loadCommandFile( "default-binds.cfg" );
		}

		//TEST ROMS - load here
		//TODO what happens if no rom is loaded - test and handle this case

		//test invalid file name
		//consoleSystem.executeRequest( &string( "loadnesfile" ), &string( "/__" ), false );
		
		consoleSystem.executeRequest( "loadnesfile", "mario" , false );
		//consoleSystem.executeRequest( "loadnesfile", "contra", false );
		//consoleSystem.executeRequest( "loadnesfile", "icehock", false );
		//consoleSystem.executeRequest( &string( "loadnesfile" ), &string( "/contra" ), false );
		//consoleSystem.executeRequest( &string( "loadnesfile" ), &string( "/castlevania" ), false );
		//consoleSystem.executeRequest( &string( "loadnesfile" ), &string( "../RomSource/ppu_test001/ppuTest001" ), false );
		
		//FAIL these
		
		
		//NESTRESS
		//consoleSystem.executeRequest( &string( "loadnesfile /nestress"), false );	
		
		//PPU tests

		//PASSED
	
		//consoleSystem.executeRequest( &string( "loadnesfile /1.Branch_Basics"), false );
		//consoleSystem.executeRequest( &string( "loadnesfile /2.Backward_Branch"), false );
		//consoleSystem.executeRequest( &string( "loadnesfile /3.Forward_Branch"), false );

		//consoleSystem.executeRequest( &string( "loadnesfile /cpu_timing_test6/cpu_timing_test"), false );	
		//consoleSystem.executeRequest( &string( "loadnesfile /blargg_ppu_tests_2005.09.15b/sprite_ram"), false );	
		//consoleSystem.executeRequest( &string( "loadnesfile /blargg_ppu_tests_2005.09.15b/vram_access"), false );	
		

		//consoleSystem.executeRequest( &string( "loadnesfile /vbl_nmi_timing/vbl_nmi_timing/1.frame_basics"), false );	
		

		

		//consoleSystem.executeRequest( &string( "loadnesfile /blargg_ppu_tests_2005.09.15b/vbl_clear_time"), false );			

		//FAILED
		

		//consoleSystem.executeRequest( &string( "loadnesfile /blargg_ppu_tests_2005.09.15b/palette_ram"), false );	
		
		//consoleSystem.executeRequest( &string( "loadnesfile /sprite_hit_tests_2005.10.05/01.basics"), false );	
		//consoleSystem.executeRequest( &string( "loadnesfile /sprite_hit_tests_2005.10.05/02.alignment"), false );	
		//consoleSystem.executeRequest( &string( "loadnesfile /sprite_hit_tests_2005.10.05/03.corners"), false );	
		//consoleSystem.executeRequest( &string( "loadnesfile /sprite_hit_tests_2005.10.05/04.flip"), false );	
		//consoleSystem.executeRequest( &string( "loadnesfile /sprite_hit_tests_2005.10.05/08.double_height"), false );	
		//consoleSystem.executeRequest( &string( "loadnesfile /sprite_hit_tests_2005.10.05/05.left_clip"), false );	
		//consoleSystem.executeRequest( &string( "loadnesfile /sprite_hit_tests_2005.10.05/06.right_edge"), false );	
		//consoleSystem.executeRequest( &string( "loadnesfile /sprite_hit_tests_2005.10.05/07.screen_bottom"), false );	
		
		
		//consoleSystem.executeRequest( &string( "loadnesfile /sprite_hit_tests_2005.10.05/09.timing_basics"), false );	
		//consoleSystem.executeRequest( &string( "loadnesfile /sprite_hit_tests_2005.10.05/10.timing_order"), false );	
		//consoleSystem.executeRequest( &string( "loadnesfile /sprite_hit_tests_2005.10.05/11.edge_timing"), false );	

		//consoleSystem.executeRequest( &string( "loadnesfile /sprite_hit_timing/sprite_hit_timing"), false );	

		//UNTESTED
		
		//PPU TESTS
		
		//consoleSystem.executeRequest( &string( "loadnesfile /blargg_ppu_tests_2005.09.15b/power_up_palette"), false );	
		
		//NMI Timing tests
		//consoleSystem.executeRequest( &string( "loadnesfile /vbl_nmi_timing/vbl_nmi_timing/2.vbl_timing"), false );	
		//consoleSystem.executeRequest( &string( "loadnesfile /vbl_nmi_timing/vbl_nmi_timing/3.even_odd_frames"), false );	

		//consoleSystem.executeRequest( &string( "loadnesfile /vbl_nmi_timing/vbl_nmi_timing/4.vbl_clear_timing"), false );	
		//consoleSystem.executeRequest( &string( "loadnesfile /vbl_nmi_timing/vbl_nmi_timing/5.nmi_suppression"), false );	
		//consoleSystem.executeRequest( &string( "loadnesfile /vbl_nmi_timing/vbl_nmi_timing/6.nmi_disable"), false );	
		//consoleSystem.executeRequest( &string( "loadnesfile /vbl_nmi_timing/vbl_nmi_timing/7.nmi_timing"), false );	

		//Blargg cpu test
		//consoleSystem.executeRequest( &string( "loadnesfile /blargg_nes_cpu_test5/official"), false );	
	}
	catch( GUIElement::GUIElementInitializeException ) {
		//FIXME windows specific code
		//MessageBox( hWnd, "Error initializing gui element", "Error", MB_OK );
		exit( 0 );
	}
	
	renderer.initialize();

	if( nesMain.getState() == Emulating ) {
		//nesMain.nesDebugger.initialize();
		//nesMain.nesDebugger.setToSingleStepMode( nesMain.nesCpu.getPC() );
	}

	gui.addElement( &guiConsole );
	//gui.addElement( &frameCounter );

	//frameCounter.setOpen( true );
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
	nesMain.update();
	graphicUpdate();

	timeProfiler.stopActive( );
}

/*
==============================================
SystemMain::graphicUpdate()
==============================================
*/
void SystemMain::graphicUpdate() {
	timeProfiler.startSection( "RenFrame" );
	renderer.renderFrame();
	

	timeProfiler.stopActive( );
}

