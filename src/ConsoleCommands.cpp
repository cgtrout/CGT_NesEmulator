#include "precompiled.h"

#include "ConsoleCommands.h"
#include "SystemMain.h"

#include <algorithm>

using namespace Console;

//Console Commands
//###formatignore	
ConsoleCommand commands[] = 
//	Name			 Handler								Description
{ { "quit",			 &CommandHandlerSystem::quit,			"Closes the program" },
  {	"loadnesfile",	 &CommandHandlerSystem::loadNesFile,    "Loads an nes file" }, 
#ifndef LIGHT_BUILD
  { "saveTrace",	 &CommandHandlerSystem::printTraceLog,	"Writes a cpu trace log" },
  { "saveAsm",		 &CommandHandlerSystem::printAsm,		"Writes dissasembled sorted trace" },

  { "startTrace",	 &CommandHandlerSystem::startTrace,	    "Starts a cpu trace." },
  { "stopTrace",	 &CommandHandlerSystem::stopTrace,		"Stops a running cpu trace." },
#endif 
  { "bind",			 &CommandHandlerSystem::bindKey,		"Binds a key to a controllable element."},
  { "reset",		 &CommandHandlerSystem::reset,			"Resets the nes cpu",					},
  { "help",			 &CommandHandlerSystem::help,			"This command"},
 
  { "END_OF_LIST",	nullptr							    } };
//###endformatignore

CommandHandlerSystem::CommandHandlerSystem() {
}

ConsoleCommand *CommandHandlerSystem::getCommands() {
	return commands;
}

void CommandHandlerSystem::quit( std::string_view  ) {
	FrontEnd::SystemMain::getInstance( )->quitRequest();
}

void CommandHandlerSystem::loadNesFile( std::string_view param ) {
	using namespace FrontEnd;

	//late binding to avoid singleton initialization hell
	if(consoleSystem == nullptr ) {
		consoleSystem = &SystemMain::getInstance( )->consoleSystem;
	}

	if( param.empty() ) {
		consoleSystem->printMessage( "No filename entered." );
		return;
	}

	try {
		//call loadNesFile function to load
		SystemMain::getInstance( )->loadNesFile( param );
		return;
	}
	catch( NesEmulator::NesFile::NesFileException e ) {
		consoleSystem->printMessage( "Error loading nes file: %s", e.getMessage() );
	}
}

#ifndef LIGHT_BUILD
using namespace FrontEnd;

void CommandHandlerSystem::printTraceLog( std::string_view param ) {
	SystemMain::getInstance( )->nesMain.nesCpu.cpuTrace.printTrace( param );
}

void CommandHandlerSystem::printAsm( std::string_view param ) {
	SystemMain::getInstance( )->nesMain.nesCpu.cpuTrace.printAsm( param );
}

void CommandHandlerSystem::startTrace( std::string_view param ) {
	SystemMain::getInstance( )->nesMain.nesCpu.cpuTrace.startTrace();
}

void CommandHandlerSystem::stopTrace( std::string_view param ) {
	SystemMain::getInstance( )->nesMain.nesCpu.cpuTrace.stopTrace();
}

#endif

//	bind controller01 to VK_A
void CommandHandlerSystem::bindKey( std::string_view param ) {
	if( param.empty() ) {
		printBindKeyUsage( "No params entered." );
		return;
	}

	std::string p( param );
		
	//get tokens
	CgtLib::StringTokenizer st;
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
	std::string device;
	std::string control;
	std::string button;
	std::string key = tokens.at( 2 );

	//command example: controller1.a
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

	tokens = st.tokenize( key );
	
	//device is keyboard or joystick id
	device = tokens.at( 0 );
	
	if ( tokens.size() > 1 ) {
		key = tokens.at( 1 );
	} else {
		printBindKeyUsage( "Invalid key entered" );
		return;
	}

	//tokens are parsed, now interpret key and command
	auto* inputSystem = &FrontEnd::SystemMain::getInstance( )->input;
	bool result = inputSystem->bindKeyToControl( device, key, control, button );
	if( result ) {
		consoleSystem->printMessage( "Key successfully bound" );
	}
}

void CommandHandlerSystem::printBindKeyUsage( std::string_view errorMsg ) {
	consoleSystem->printMessage( "Invalid bind command: %s", errorMsg );
	consoleSystem->printMessage( "Usage of bind:  \"bind controller.command to device.key\"" );
	consoleSystem->printMessage( "For example: \"bind controller1.a to keyboard.z\"");
}

void CommandHandlerSystem::reset( std::string_view ) {
	using namespace FrontEnd;

	consoleSystem->printMessage( "Resetting cpu..." );
	auto* systemMain = SystemMain::getInstance( );
	systemMain->nesMain.nesCpu.reset();
	systemMain->nesMain.reset();
}

//help syntax: help 
void CommandHandlerSystem::help( std::string_view param ) {
	consoleSystem->printMessage( "--------------------------------------------------------------" );
	consoleSystem->printMessage( "VARIABLES LIST" );
	consoleSystem->printMessage( "--------------------------------------------------------------" );

	std::list<std::string*> variableList;
	consoleSystem->variables.getNameList( &variableList );

	// Sort the list using a custom comparator for string pointers
	variableList.sort( []( const std::string* a, const std::string* b ) {
		return ( *a < *b );
	});
	std::ostringstream ss2;
	for( const auto& v : variableList ) {
		ss2 << *v << std::endl;
	}
	
	consoleSystem->printMessage( ss2.str( ).c_str( ) );

	consoleSystem->printMessage( "--------------------------------------------------------------" );
	consoleSystem->printMessage( "COMMANDS LIST" );
	consoleSystem->printMessage( "--------------------------------------------------------------" );

	/// Create a copy of the vector and then sort
	std::vector<ConsoleCommand*> newVector = consoleSystem->commands;

	// Sort the new vector using a custom comparator
	std::sort( newVector.begin( ), newVector.end( ),
		[]( const ConsoleCommand* a, const ConsoleCommand* b ) {
			return ( a->name < b->name );
		} );

	std::ostringstream ss;
	for( const auto& v : newVector ) {
		ss << std::left << std::setw( 15 ) << v->name << " "
			<< std::left << std::setw( 20 ) << v->description
			<< std::endl;
	}

	consoleSystem->printMessage( ss.str( ).c_str( ) );
	consoleSystem->printMessage( "" );


	consoleSystem->printMessage( "Press PAGEUP to view all of help" );
	consoleSystem->printMessage( "--------------------------------------------------------------" );
}

void CommandHandlerSystem::printHelpUsage( std::string_view errorMsg ) {
	if ( errorMsg != nullptr ) {
		consoleSystem->printMessage( "Invalid help command: %s", errorMsg );
	}
	consoleSystem->printMessage( "Usage of help: \"help [ command/variable ]\".  This will print " );
	consoleSystem->printMessage( "a description of the command or variable to the console" );
}


