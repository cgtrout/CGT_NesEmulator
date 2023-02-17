// GameMain.cpp: implementation of the GameMain class.
//
//////////////////////////////////////////////////////////////////////

#include "GameMain.h"
#include < math.h >
#include "NesDebugger.h"

using namespace GUISystem;

//#include "GUIConsole.h"
#include "Console.h"
extern Console::ConsoleSystem *consoleSystem;

/*
==============================================
GameMain::GameMain( )
==============================================
*/
GameMain::GameMain( ) {
	renderer = new Renderer( );
	
	gui = new GUI( );

	try {
		gc = new GUIConsole( );
		gui->addElement( gc );
		
		gui->setUsingMouse( true );
	}
	catch( GUIElement::GUIElementInitializeException ) {
		MessageBox( hWnd, "Error initializing gui element", "Error", MB_OK );
		exit( 0 );
	}
	
	wantsQuit = false;
	totalElapsedTime = 0.0f;
	frameNumber = 0.0f;
	currTime = 0;
	elapsedTime = 0;
	totalElapsedTime = 0;

	input->setUseDelay( true );
	nesDebugger = NesDebugger::getInstance( );
}

/*
==============================================
GameMain::~GameMain( )
==============================================
*/
GameMain::~GameMain( ) {
	//delete level;
	delete gui;
}

/*
==============================================
GameMain::startGame( )
==============================================
*/
void GameMain::startGame( ) {
	//start timer
	timer->init( );
	renderer->initialize( );
}

/*
==============================================
GameMain::runFrame( )
==============================================
*/
void GameMain::runFrame( ) {
	renderer->initFrame( );
	
	//update timer
	elapsedTime = timer->getCurrTime( ) - currTime;
	currTime = timer->getCurrTime( );

	//for fps calc
	frameNumber += 1;
	totalElapsedTime += elapsedTime;

	graphicUpdate( );

	//see if escape key is down for quitting
	if( input->isKeyDown( KB_ESCAPE ) ) {
		wantsQuit = true;
		nesCpu->cpuTrace.printTrace( );
	}
	if( input->isKeyDown( KB_TILDE ) ) {
		gc->setOpen( !gc->isOpen( ) );
	}
	
	//F5 controls single stepping with the debugger
	if( input->isKeyDown( KB_F5 ) ) {
		if( !nesDebugger->inSingleStepMode( ) ) {
			nesDebugger->setToSingleStepMode( nesCpu->getPC( ) );
		}
		else {
			//request that a single step is done in debugger
			nesDebugger->singleStepRequest( );
		}
	}
	if( input->isKeyDown( KB_F9 ) ) {
		nesDebugger->addBreakPoint( nesDebugger->getSelectedAddress( ) );
	}
	if( input->isKeyDown( KB_RETURN ) ) {
		if( nesDebugger->isOpen( ) ) {
			nesDebugger->onEnter( );
		}
	}
	
	gui->runFrame( );
	nesCpu->update( );
}

/*
==============================================
GameMain::graphicUpdate( )
==============================================
*/
void GameMain::graphicUpdate( ) {
	try {
		gui->render( );
	}
    	
	catch( GUI::GUIRunException ) {
		MessageBox( hWnd, "GUIRunException caught", "Error", MB_OK );
		exit( 0 );
	}
	
	renderer->renderFrame( );
}

