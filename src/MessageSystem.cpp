#pragma warning( disable : 4786 )

#include "MessageSystem.h"

extern MessageSystem *ms;

void MessageSystem::runCommand( std::string line, bool echo ) {
	parseLine( line );
	
	if( echo ) {
		history += " > ";
		history += line.c_str( );
		history += "\n";
	}
	
	//find category                                          
	bool catFound = false;
	MessageCategory *currCat = NULL;
	std::list< MessageCategory* >::iterator catiter;
	for( catiter = categories.begin( ); catiter != categories.end( ); catiter++ ) {
		currCat = ( MessageCategory* )( *catiter );
		if( currCat->getStringForm( ).compare( category ) == 0 ) {
			catFound = true;
			break;
		}
	}
	if( !catFound ) {
		history += "Category \"";
		history += category.c_str( );
		history += "\" not found";
		history += "\n";
		return;
	}

	//find command
	bool comFound = false;

	std::list< MessageCommand* > commands = currCat->getCommands( );
	MessageCommand *currCom = NULL;
	std::list< MessageCommand* >::iterator comiter;
	for( comiter = commands.begin( ); comiter != commands.end( ); comiter++ ) {
		currCom = ( MessageCommand* )( *comiter );
		if( currCom->getStringForm( ).compare( command ) == 0 ) {
			comFound = true;
			break;
		}
	}
	if( !comFound ) {
		history += "Command \"";
		history += command.c_str( );
		history += "\" not found";
		history += " in category \"";
		history += category.c_str( );
		history += "\"\n";
		return;
	}
	
	//run command
	currCom->run( args, true );
	category = "";
	command = "";
	args = "";
}

void MessageSystem::parseLine( std::string line ) {
	int periodPos = -1;
	unsigned int x;
	for( x = 0; x < line.length( ); x++ ) {
		if( line[ x ] == ' ' ) {
			break;
		}
		if( line[ x ] == '.' ) {
			periodPos = x;
			break;
		}
	}
	if( periodPos != -1 ) {
		category = line.substr( 0, periodPos );
	}	
	else {
		//period was not found; assume that the category is 'root'
		category = "root";
		x = 0;
	}
	int spacePos = -1;
	for( ; x < line.length( ); x++ ) {
		if( line[ x ] == ' ' ) {
			spacePos = x;
			break;
		}
	}
	if( spacePos == -1 ) {
		command = line.substr( periodPos+1, ( line.length( )-1 ) - periodPos );
	}
	else {
		if( periodPos != -1 ) {
			command = line.substr( periodPos+1, line.length( ) - periodPos - ( line.length( ) - spacePos ) - 1 );
			args = line.substr( spacePos+1, line.length( ) - spacePos - 1 );
		}
		else {
			command = line.substr( 0, spacePos );
			args = line.substr( spacePos+1, line.length( ) - spacePos -1 );
		}
	}
}

void MessageVariable::run( std::string args, bool echo ) {
	std::string command = "print ";
	
	//if no arg is given simply print the variables value
	if( args.length( ) == 0 ) {
		command += "Value is \'";
		command += strvalue.c_str( );
		command += "\' ";
	}
	//else take value given in args and assign it as the new value
	else {
		switch( type ) {
		case MV_INT:
			try {
				intvalue = convertStrToInt( args );
				strvalue = args;
			}
			catch( StringToNumberException ) {
				command += "Invalid value \'";
				command += args.c_str( );
				command += "\'";
				goto mvrunend;
			}
			break;
		case MV_FLOAT:
			//TODO implement
			break;
		}
		command += "Value set to \'";
		command += strvalue.c_str( );
		command += "\' ";
	}
mvrunend:

	if( echo ) {
		ms->runCommand( command, false );
	}
}

void MessageSystem::MCatRoot::MComTestCommand::run( std::string args, bool echo ) {
	ms->runCommand( "print root.testcommand", false );
}

void MessageSystem::MCatRoot::MComPrint::run( std::string args, bool echo ) {
	//history += "\""
	//history += args;
	//history += "\"\n"
	//ms->printString( "\"" );
	ms->printString( args );
	//ms->printString( "\"\n" );
}

//root.quit command
void MessageSystem::MCatRoot::MComQuit::run( std::string args, bool echo ) {
	gm->setWantsQuit( true );
}


//nes.load command
void MessageSystem::MCatNes::MComLoad::run( std::string args, bool echo ) {
	if( args.length( ) == 0 ) {
		ms->printString( "a filename must be specified" );
		return;
	}

	args += ".nes";
	
	try {
		nesCpu->getNesFile( )->loadFile( args );
		ms->printString( "nes file load successful" );
		
		nesCpu->reset( );
		nesCpu->setOnStatus( true );
	}
	catch( NesFile::NesFileException e ) {
		ms->printString( "error loading file" );
	}
}

void MessageSystem::MCatNes::MComDebug::run( std::string args, bool echo ) {
    if( nesDebugger == NULL ) {
        nesDebugger = NesDebugger::getInstance( );
    }
    nesDebugger->setToSingleStepMode( nesCpu->getPC( ) );
}

//[ any_category ].list
void MessageCategory::ListCommand::run( std::string args, bool echo ) {
	//need to go through commands in list
	std::list< MessageCommand* > commands = lccommands;
	MessageCommand *currCom = NULL;
	std::list< MessageCommand* >::iterator comiter;
	for( comiter = commands.begin( ); comiter != commands.end( ); comiter++ ) {
		currCom = ( MessageCommand* )( *comiter );
		ms->printString( currCom->getStringForm( ) );
	}
}
