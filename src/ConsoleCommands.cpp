#include "precompiled.h"

using namespace Console; 
using namespace FrontEnd;
using namespace NesEmulator;

#if _MSC_VER > 1000
#pragma warning ( disable : 4996 )
#endif

//Console Commands
//###formatignore	
ConsoleCommand commands[] = 
//	Name			 Handler								Description
{ { "quit",			 &CommandHandlerSystem::quit,			"Closes the program " },
  {	"loadnesfile",	 &CommandHandlerSystem::loadNesFile,    "Loads an nes file: usage \"loadNesFile [filename]\" " }, 
#ifndef LIGHT_BUILD
 // { "writeTrace",	 &CommandHandlerSystem::printTraceLog,	"Writes a cpu trace log: usage \"printTraceLog [filename]\"" },
 // { "writeAsm",		 &CommandHandlerSystem::printAsm,		"Writes dissasembled sorted trace: usage \"printTraceLog [filename]\"" },


 // { "startTrace",	 &CommandHandlerSystem::startTrace,	    "Starts a cpu trace." },
 // { "stopTrace",	 &CommandHandlerSystem::stopTrace,		"Stops a running cpu trace." },
#endif 
  { "bind",			 &CommandHandlerSystem::bindKey,		"Binds a key to a controllable element."},
  { "reset",		 &CommandHandlerSystem::reset,			"Resets the nes cpu",					},
  { "help",			 &CommandHandlerSystem::help,			"Allows viewing of console variable and command descriptions"},
 
  { "END_OF_LIST",	NULL							    } };
//###endformatignore

CommandHandlerSystem::CommandHandlerSystem() {
	consoleSystem = nullptr;
}

ConsoleCommand *CommandHandlerSystem::getCommands() {
	return commands;
}

void CommandHandlerSystem::quit( const char *param ) {
	systemMain->quitRequest();
}

void CommandHandlerSystem::loadNesFile( const char *param ) {
	//late binding to avoid singleton initialization hell
	if ( consoleSystem == nullptr ) {
		consoleSystem = &SystemMain::getInstance( )->consoleSystem;
	}

	if( !param ) {
		consoleSystem->printMessage( "No filename entered." );
		return;
	}

	try {
		//TODO move to seperate function in nesmain
		//systemMain->nesMain.nesMemory.zeroMemory();
		//systemMain->nesMain.nesMemory.ppuMemory.zeroMemory();
		//restart sound system if it is already running
		if( systemMain->soundSystem->isInitialized() ) {
			systemMain->soundSystem->shutDown();
		}
		consoleSystem->printMessage( "Initializing soundsystem" );

		//start sound system
		try {
			systemMain->soundSystem->initialize();
			systemMain->soundSystem->start();
			//consoleSystem->printMessage( "Soundsystem started" );
		} catch( Sound::SoundSystemException e ) {
			consoleSystem->printMessage( "Soundsystem start failed - %s", e.getMessage().c_str() );
		}
		
		systemMain->nesMain.setState( WaitingForFile );
		systemMain->nesMain.nesFile.loadFile( param );
		consoleSystem->printMessage( "NES file load successful" );
        
		systemMain->nesMain.nesPpu.reset();
		systemMain->nesMain.reset();
		systemMain->nesMain.nesCpu.reset();
		//systemMain->nesMain.nesCpu.setOnStatus( true );
		systemMain->nesMain.setState( Emulating );
		return;
	}
	catch( NesFile::NesFileException *e ) {
		consoleSystem->printMessage( "Error loading nes file: %s", e->getMessage() );
	}
}

#ifndef LIGHT_BUILD
/*
void CommandHandlerSystem::printTraceLog( const char *param ) {
	systemMain->nesMain.nesCpu.cpuTrace.printTrace( param );
}

void CommandHandlerSystem::printAsm( const char *param ) {
	systemMain->nesMain.nesCpu.cpuTrace.printAsm( param );
}

void CommandHandlerSystem::startTrace( const char *param ) {
	systemMain->nesMain.nesCpu.cpuTrace.startTrace();
}

void CommandHandlerSystem::stopTrace( const char *param ) {
	systemMain->nesMain.nesCpu.cpuTrace.stopTrace();
}
*/
#endif

//	bind controller01 to VK_A
using namespace CgtString;
void CommandHandlerSystem::bindKey( const char *param ) {
	if( param == NULL ) {
		printBindKeyUsage( "No params entered." );
		return;
	}

	std::string p = param;
		
	//get tokens
	StringTokenizer st;
	st.setDelims( " " );
	auto tokens = st.tokenize( p );
	
	if( tokens.size() < 3 ) {
		printBindKeyUsage( "Not enough params entered." );
		return;
	}
	
	if( tokens.size() > 3 ) {
		printBindKeyUsage( "Too many params entered." );
		return;
	}
	if( tokens.at( 1 ) != "to" ) {
		printBindKeyUsage( "Param 2 must be \"to\"" );
		return;
	}

	//parse command
	std::string control;
	std::string button;
	std::string key = tokens.at( 2 );

	std::string command = tokens.at( 0 );
	
	//tokenize command using '.' as delimiter
	st.setDelims( "." );
	tokens = st.tokenize( command );

	if( tokens.size() < 2 ) {
		printBindKeyUsage( "Invalid command entered" );
		return;
	}
	control = tokens.at( 0 );
	button = tokens.at( 1 );

	
	//tokens are parsed, now interpret key and command
	bool result = Input::getInstance()->bindKeyToControl( key, control, button );
	if( result ) {
		consoleSystem->printMessage( "Key successfully bound" );
	}
}

void CommandHandlerSystem::printBindKeyUsage( const char *errorMsg ) {
	consoleSystem->printMessage( "Invalid bind command: %s", errorMsg );
	consoleSystem->printMessage( "Usage of bind:  \"bind controller.command to key\"" );
}

void CommandHandlerSystem::reset( const char *param ) {
	consoleSystem->printMessage( "Resetting cpu..." );
	systemMain->nesMain.nesCpu.reset();
	systemMain->nesMain.reset();
}

//help syntax: help [ command / param ]
void CommandHandlerSystem::help( const char *param ) {
	std::string params( param );
	StringTokenizer st;
	st.setDelims( " " );
	auto tokens = st.tokenize( params );

	if( tokens.size() == 0 ) {
		printHelpUsage( NULL );
		return;
	}

	if( tokens.size() > 1 ) {
		printHelpUsage( "Too many params entered" );
		return;
	}

	//first see if it is a variable
	std::string desc = consoleSystem->variables.getVariableDescription( ( ( tokens )[ 0 ] ) ) ;
	if( desc != "NOT_FOUND" ) {
		consoleSystem->printMessage( "%s: %s", ( tokens )[ 0 ].c_str(), desc.c_str() );
		return;
	}
	
	//now see if consoleCommand can be found ( since variable was not found"
	desc = consoleSystem->getCommandDescription( ( tokens )[ 0 ] );
	if( desc != "NOT_FOUND" ) {
		consoleSystem->printMessage( "%s: %s", ( tokens )[ 0 ].c_str(), desc.c_str() );
		return;
	}
	//if here, then console or variable was not found
	printHelpUsage( "Variable or command matching given param could not be found" );
}
void CommandHandlerSystem::printHelpUsage( const char *errorMsg ) {
	if ( errorMsg != NULL ) {
		consoleSystem->printMessage( "Invalid help command: %s", errorMsg );
	}
	consoleSystem->printMessage( "Usage of help: \"help [ command/variable ]\".  This will print " );
	consoleSystem->printMessage( "a description of the command or variable to the console" );
}


