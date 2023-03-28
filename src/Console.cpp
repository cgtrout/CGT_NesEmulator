#include "precompiled.h"

#include "ConsoleCommands.h"
#include "StringToNumber.h"
#include "CGString.h"

#include <sstream>
#include <cstdarg>

using namespace Console;

/* 
==============================================
void ConsoleVariable< T >::setValueString
==============================================
*/
template < class T >
void ConsoleVariable< T >::setValueString( std::string_view val ) {
	std::stringstream ss( val.data() );
	ss >> value;
}

/* 
==============================================
void ConsoleVariable< T >::setValue( T val )
==============================================
*/
template < class T >
void ConsoleVariable< T >::setValue( T val ) {
	this->value = val;
}

/* 
==============================================
std::string ConsoleVariable< T >::getValueString()
==============================================
*/
template < class T >
std::string ConsoleVariable< T >::getValueString() {
	std::stringstream ss;
	ss << this->value;
	return ss.str();
}
/* 
==============================================
Variables::Variables()
==============================================
*/
Variables::Variables() {
	variableFile.loadFile( "ConsoleDefs.txt" );
}

/* 
==============================================
Variables::~Variables()
==============================================
*/
Variables::~Variables() {
	variableFile.saveFile( "ConsoleDefs.txt", this );
}
/* 
==============================================
void Variables::addIntVariable( ConsoleVariable< int > *variable )
==============================================
*/
void Variables::addIntVariable( ConsoleVariable< int > *variable ) {
	addVariable< int >( variable, &intVars );
}
/* 
==============================================
void Variables::addFloatVariable( ConsoleVariable< float > *variable )
==============================================
*/
void Variables::addFloatVariable( ConsoleVariable< float > *variable ) {
	addVariable< float >( variable, &floatVars );
}
/* 
==============================================
void Variables::addBoolVariable( ConsoleVariable< bool > *variable )
==============================================
*/
void Variables::addBoolVariable( ConsoleVariable< bool > *variable ) {
	addVariable< bool >( variable, &boolVars );
}
/* 
==============================================
void Variables::addStringVariable( ConsoleVariable< std::string > *variable )
==============================================
*/
void Variables::addStringVariable( ConsoleVariable< std::string > *variable ) {
	addVariable< std::string >( variable, &stringVars );
}

/* 
==============================================
void Variables::addVariable( ConsoleVariable< T > *variable, list< ConsoleVariable< T >* > *varList )
==============================================
*/
template< class T >
void Variables::addVariable( ConsoleVariable< T > *variable, std::list< ConsoleVariable< T >* > *varList ) {
	//gothrough all lists and make sure that variable does not exist
	if( getValueString( *variable->getName() ) != "NOT_FOUND" ) {
		std::stringstream ss;
		ss << "Variable: " << *variable->getName() << " already exists";
		throw ConsoleSystemException( "Variables::addVariable", ss.str().c_str(), true );
	}
	
	varList->push_back( variable );
	
	VariableFile::DefinitionLine *def;
	
	//check variable define file to see if a variable by this name exists
	def = variableFile.getDefinition( *variable->getName() );
	
	//if def was found
	if( def != NULL ) {
		//set values to values that are in file
		variable->setValueString( def->valstr );
	}
}


ConsoleVariable< int > *Variables::getIntVariable( std::string_view variable ) {
	return getVariable< int >( variable, &intVars );
}

ConsoleVariable< float > *Variables::getFloatVariable( std::string_view variable ) {
	return getVariable< float >( variable, &floatVars );
}


ConsoleVariable< bool > *Variables::getBoolVariable(  std::string_view variable ) {
	return getVariable< bool >( variable, &boolVars );
}


ConsoleVariable< std::string > *Variables::getStringVariable( std::string_view variable ) {
	return getVariable< std::string >( variable, &stringVars );
}

/* 
==============================================
Variables::getVariable
==============================================
*/
template< class T >
ConsoleVariable< T > *Variables::getVariable( std::string_view name, std::list< ConsoleVariable< T >* > *varList ) {
	//go through entire varList
	typename std::list < ConsoleVariable< T >* >::iterator i;
	for( i = varList->begin(); i != varList->end(); i++ ) {
		ConsoleVariable< T > *t = *i;
		if( CgtLib::toLower( name ) == CgtLib::toLower( *t->getName() ) ) {
			return t;
		}
	}
	return NULL;
}

/* 
==============================================
std::string Variables::getValueString( std::string *varName )
==============================================
*/
std::string Variables::getValueString( std::string_view varName ) {
	//go through varlists
	ConsoleVariable< int > *i	  = getIntVariable( varName );
	ConsoleVariable< float > *f  	  = getFloatVariable( varName );
	ConsoleVariable< bool > *b   	  = getBoolVariable( varName );
	ConsoleVariable< std::string > *s = getStringVariable( varName );

	//check return variable found
	//if they are not null that means that we found a variable
	//if they are found get the string value
	if( i != NULL ) return i->getValueString();
	if( f != NULL ) return f->getValueString();
	if( b != NULL ) return b->getValueString();
	if( s != NULL ) return s->getValueString();
	
	//if all are null then varName does not exist
	return "NOT_FOUND";
}

/* 
==============================================
std::string Variables::getVariableDescription( std::string_view varName )
==============================================
*/
std::string Variables::getVariableDescription( std::string_view varName ) {
	//go through varlists
	ConsoleVariable< int > *i	  = getIntVariable( varName );
	ConsoleVariable< float > *f 	  = getFloatVariable( varName );
	ConsoleVariable< bool > *b   	  = getBoolVariable( varName );
	ConsoleVariable< std::string > *s = getStringVariable( varName );

	//check return variable found
	//if they are not null that means that we found a variable
	//if they are found get the string value
	if( i != NULL ) return std::string( *i->getDescription() );
	if( f != NULL ) return std::string( *f->getDescription() );
	if( b != NULL ) return std::string( *b->getDescription() );
	if( s != NULL ) return std::string( *s->getDescription() );
	
	//if all are null then varName does not exist
	return "NOT_FOUND";
}

/* 
==============================================
bool Variables::getSaveVarToFile( std::string_view varName )
==============================================
*/
bool Variables::getSaveVarToFile( std::string_view varName ) {
	std::string retVal;
	std::stringstream ss;
	//go through varlists
	ConsoleVariable< int > *i	  = getIntVariable( varName );
	ConsoleVariable< float > *f  	  = getFloatVariable( varName );
	ConsoleVariable< bool > *b   	  = getBoolVariable( varName );
	ConsoleVariable< std::string > *s = getStringVariable( varName );

	//check return variable found
	//if they are not null that means that we found a variable
	//if they are found get the saveTo value
	if( i != NULL ) return i->getSaveToFile();
	if( f != NULL ) return f->getSaveToFile();
	if( b != NULL ) return b->getSaveToFile();
	if( s != NULL ) return s->getSaveToFile();
	
	//if all are null then varName does not exist
	return false;
}

/* 
==============================================
void Variables::getNameList( list< ConsoleVariable< T >* > *genericVarList, std::list< string* > *stringVector )
==============================================
*/
template< class T >
void Variables::getNameList( std::list< ConsoleVariable< T >* > *genericVarList, std::list< std::string* > *stringVector ) {
	typename std::list < ConsoleVariable< T >* >::iterator i;
	for( i = genericVarList->begin(); i != genericVarList->end(); i++ ) {
		auto& v = *i;
		auto name = v->getName( );
		stringVector->push_back( name );
	}
}

/* 
==============================================
void Variables::getNameList( std::list< string* > *results )
==============================================
*/
void Variables::getNameList( std::list< std::string* > *results ) {
	//all this does is go though all of the var lists and puts the names
	//of the vars into a list of strings
	
	//go through all var types
	getNameList( &intVars, results );
	getNameList( &floatVars, results );
	getNameList( &boolVars, results );
	getNameList( &stringVars, results );
}

/* 
==============================================
ConsoleSystem::ConsoleSystem() : requestPos( 0 )
==============================================
*/
ConsoleSystem::ConsoleSystem() : requestPos( 0 ) {
	commandHandlerSystem = CommandHandlerSystem::getInstance();	
}

/* 
==============================================
ConsoleSystem::~ConsoleSystem()
==============================================
*/
ConsoleSystem::~ConsoleSystem() {
}

/* 
==============================================
void ConsoleSystem::initialize()
==============================================
*/
void ConsoleSystem::initialize() {
	//get commands declared in CommandHandlerSystem.cpp
	ConsoleCommand* localCommands = commandHandlerSystem->getCommands();

	//add commands to list
	while( localCommands->name != "END_OF_LIST" ) {
		addCommand( localCommands++ );
	}
}

/* 
==============================================
void ConsoleSystem::addCommand( ConsoleCommand *command
==============================================
*/
void ConsoleSystem::addCommand( ConsoleCommand *command ) {
	commands.push_back( command );
}

/* 
==============================================
void ConsoleSystem::executeRequest( std::string_view str, bool echo = true )
==============================================
*/
void ConsoleSystem::executeRequest( std::string_view str, bool echo = true ) {
	//tokenize strings
	CgtLib::StringTokenizer st;
	st.setMaxTokens( 2 );
	st.setMinTokens( 2 );
	auto strings = st.tokenize( str );
	
	executeRequest( strings.at( 0 ) , strings.at( 1 ), echo );
}

/* 
==============================================
void ConsoleSystem::executeRequest
==============================================
*/
void ConsoleSystem::executeRequest( const std::string &command, const std::string &value, bool echo ) {
	if( command.empty() == true ) {
		return;
	}
	if( echo ) {
		//echo command
		printMessage( " > %s %s", command.data(), value.data() );
		std::stringstream ss;
		ss << command;
		
		//add to previous request history list
		if( value.empty() != true ) {
			ss << " " << value;
		}
		//ss << ends;
		prevRequests.push_front( ss.str() );
		requestPos = 0;
	}
	
	//is the command a variable
	if( findAndRunVariable( command, value ) ) {
        return; //was a variable : we are done
    }
    
	//next check to see if it is a command
	if( findAndRunCommand( command, value ) ) {
		return; //found command : we are done
	}

	//print error message
	printMessage( """%s %s"" was not recognized", command.c_str(), value.c_str() );
}


/* 
==============================================
bool ConsoleSystem::findAndRunVariable( std::string *varName, std::string *value, bool run )
==============================================
*/
bool ConsoleSystem::findAndRunVariable( std::string_view varName, std::string_view value, bool run ) {
    //go through variables until varName is found
	ConsoleVariable< int >    *i 		= variables.getIntVariable( varName );
	ConsoleVariable< float >  *f 		= variables.getFloatVariable( varName );
	ConsoleVariable< bool >   *b 		= variables.getBoolVariable( varName );
	ConsoleVariable< std::string > *s 	= variables.getStringVariable( varName );
	
	if( i == NULL && f == NULL && b == NULL && s == NULL ) {
		//not found so return false
		return false;
	}

	if( run  && value.length() != 0 ) {
		if( i != NULL ) i->setValueString( value );
		if( f != NULL ) f->setValueString( value );
		if( b != NULL ) b->setValueString( value );
		if( s != NULL ) b->setValueString( value );
	}
	
	std::stringstream ss;
	ss << "Variable \"";

	if( i != NULL ) ss << *i->getName() << "\" is set to: " << i->getValue();
	if( f != NULL ) ss << *f->getName() << "\" is set to: " << f->getValue();
	if( b != NULL ) ss << *b->getName() << "\" is set to: " << b->getValue();
	if( s != NULL ) ss << *s->getName() << "\" is set to: " << s->getValue();

	printMessage( "%s", ss.str().c_str() );

	return true;
}


/* 
==============================================
bool ConsoleSystem::findAndRunCommand( std::string *commandName, std::string *param, bool run
==============================================
*/
bool ConsoleSystem::findAndRunCommand( std::string_view commandName, std::string_view param, bool run ) {
	ConsoleCommand *curr;
	
	//go through list of console commands to try and find a match
	
	for( auto iter = commands.begin( ); iter != commands.end( ); iter++ ) {
		curr = ( ConsoleCommand* )( *iter );

		if( CgtLib::stringCaseCmp( curr->name.c_str(), commandName ) ) {
			if( run ) {
				runCommand( curr, param );
			}
			return true;
		}
	}
	//not found
	return false;
}

/* 
==============================================
ConsoleSystem::runCommand
==============================================
*/
void ConsoleSystem::runCommand( ConsoleCommand *command, std::string_view param ) {
	auto trimmed = CgtLib::trimWhitespace( param );
	
	//runs handler function using a member function pointer
	( commandHandlerSystem->*command->handler )( trimmed );
}

/* 
==============================================
ConsoleSystem::getCommandDescription
==============================================
*/
std::string ConsoleSystem::getCommandDescription( std::string_view commandName ) {
	for( auto& i = commands.begin(); i != commands.end(); i++ ) {
		ConsoleCommand *c = ( ConsoleCommand* )( *i );
		if( CgtLib::toLower( commandName ) ==  CgtLib::toLower( c->name ) ) {
			return c->description;
		}
	}
	return "NOT_FOUND";
}

/* 
==============================================
void ConsoleSystem::printMessage( const char *message, ...

TODO convert to c++ strings
==============================================
*/
void ConsoleSystem::printMessage( const char* message, ... ) {
	// Create a buffer and format the message
	const int bufferSize = 1024;
	std::vector<char> buffer( bufferSize );
	va_list argptr;
	va_start( argptr, message );
	vsnprintf( buffer.data( ), bufferSize, message, argptr );
	va_end( argptr );

	// Convert the buffer to a std::string
	std::string formattedMessage( buffer.data( ) );

	// Create a stringstream to hold the formatted message
	std::stringstream ss( formattedMessage );

	// Split the formatted string into lines and write each line to the history
	std::string line;
	while( std::getline( ss, line ) ) {
		if( line.size( ) > MaxLineSize ) {
			// Chop up long lines
			for( size_t i = 0; i < line.size( ); i += MaxLineSize ) {
				std::string substr = line.substr( i, MaxLineSize );
				history.writeLine( substr.c_str( ) );
			}
		}
		else {
			history.writeLine( line.c_str( ) );
		}
	}
}

/* 
==============================================
std::string ConsoleSystem::getHistoryLines( int numLinesToGet, int pos )
  get the last 'numLines' number of lines printed to console hi
==============================================
*/
std::string ConsoleSystem::getHistoryLines( int numLinesToGet, int pos ) {
    static int lastPos = 0;

	if( history.numLines < numLinesToGet - pos ) {
		return history.getBuffer( 0, history.numLines );
	}
	if( history.numLines == 0 )
	 {
		return "";
	}
	int startLine = history.numLines - numLinesToGet - pos;
	return history.getBuffer( startLine, startLine + numLinesToGet );
}

/* 
==============================================
ConsoleSystem::History::getBuffer( int startline, int endline )
==============================================
*/
std::string ConsoleSystem::History::getBuffer( int startline, int endline ) {
	std::string returnBuffer;
    
    for( auto i = startline; i < endline && i < numLines; i++ ) {
        auto line = getLine( i );
		returnBuffer += getLine( i );
		returnBuffer += "\n";
    }
	return returnBuffer;
}

/* 
==============================================
void ConsoleSystem::History::writeLine(  )
==============================================
*/
void ConsoleSystem::History::writeLine( std::string_view string ) {
    //write to current line
	lines[ currentLine ].text = string;
    currentLine++;
        
	if( numLines < MaxLines ) {
        numLines++;
    }
    
    //if next line greater than MaxLines set current line to zero
    if( currentLine >= MaxLines ) {
        currentLine = 0;
    }
}

/* 
==============================================
ConsoleSystem::History::getLine( int lineToGet )
==============================================
*/
std::string ConsoleSystem::History::getLine( int lineToGet ) {
    int firstLinePos, offset, pos;
    
    if( lineToGet >= numLines ) {
		return "";
    }
    
    //calculate where first line is
    if( numLines == MaxLines ) {
        firstLinePos = currentLine;
    }
    else { 
		//have not done wrap around yet so it must be zero
        firstLinePos = 0;
    }
    
    //calculate offset
    offset = firstLinePos + lineToGet;
    if( offset >= MaxLines ) {
        pos = offset - MaxLines;
    }
	else {
		pos = offset;
	}
    return lines[ pos ].text;
}

/* 
==============================================
ConsoleSystem::printMatches(  )
  used to show possible commands for user when they hit tab with a partial command
  returns match if only one found
  otherwise it returns NULL

TODO return string that matches given partial until the point where the matches no longer match up
==============================================
*/
std::string ConsoleSystem::printMatches( std::string_view partial ) {
	std::list< std::string* > matchList{};

	int partialLength = partial.length();
	int matchesFound = 0;
	std::string lastMatch{};	//last match found string
	
	if( partialLength == 0 ) {
		return std::string();
	}

	//convert to lowercase
	std::string plcase = CgtLib::strtolower( partial );
	
	//loop through all commands
	for( auto &ccur : commands ) {
		std::string compstr = CgtLib::strtolower( ccur->name );
		
		//if current command's name == partial
		if( compstr.compare( 0, partialLength, plcase ) == 0 ) {
			//partial match made so add it to list
			matchList.push_back( &ccur->name );			
			matchesFound++;
			lastMatch = ccur->name;
		}	
	}

	//todo use safe ptr so that nameList gets deleted
	std::list< std::string* > nameList;
	variables.getNameList( &nameList );
	nameList.sort();

	//now go through all of the variables
	std::list< std::string* >::iterator viter;
	std::string* cvar;
	for( viter = nameList.begin(); viter != nameList.end(); viter++ ) {
		cvar = ( std::string* )( *viter );
		std::string compstr = CgtLib::strtolower( cvar->c_str() );
		
		//if current variables name == partial
		if( strncmp( compstr.c_str(), plcase.c_str(), partialLength ) == 0 ) {
			//partial match made so add it to list
			matchList.push_back( cvar );
			matchesFound++;
			lastMatch = *cvar;
		}
	}
	//see if only one match was found
	if( matchesFound == 1 ) { 
		//return string result
		return lastMatch;
	}

	//sort the list
	matchList.sort();

	//traverse through sorted list and print results
	std::list< std::string* >::iterator siter;
	std::string *cstr;
	for( siter = matchList.begin(); siter != matchList.end(); siter++ ) {
		cstr = ( std::string* )( *siter );
		printMessage( cstr->c_str() );
	}
	//more than one match found so return null
	return std::string( "" );
}

/* 
==============================================
std::string ConsoleSystem::getPreviousRequest()
==============================================
*/
std::string ConsoleSystem::getPreviousRequest() {
	std::string curr = "";
	if( prevRequests.size() == 0 ) {
		return curr;
	}
	if( requestPos == 0 ) {
		requestIter = prevRequests.begin();	
	}
	curr = ( std::string )( *requestIter );
		
	if( requestPos + 1 < prevRequests.size() ) {
		requestIter++;
		requestPos++;
	}

	return curr;
}

/* 
==============================================
std::string ConsoleSystem::getNextRequest()
==============================================
*/
std::string ConsoleSystem::getNextRequest() {
	std::string curr;
	if( requestPos == 0 ) {
		return curr;
	}

	requestIter--;
	requestPos--;
	curr = ( std::string )( *requestIter );
	return curr;
}

/* 
==============================================
void ConsoleSystem::loadCommandFile

==============================================
*/
void ConsoleSystem::loadCommandFile( std::string_view filename ) {
	std::ifstream file( filename.data( ) );
	std::string line;

	if( !file ) {
		return;
	}

	while( std::getline( file, line ) ) {
		// Comment line ignore
		if( line.substr( 0, 2 ) == "//" ) {
			continue;
		}
		// Pass command to system
		executeRequest( line.c_str(), false );
	}
}

/* 
==============================================
void VariableFile::loadFile(  )

  TODO change this to use C++ strings/streams?
==============================================
*/
void VariableFile::loadFile( std::string_view filename ) {
	std::ifstream file( filename.data( ) );
	std::string line;
	std::string valBuf;

	// Check for error. If file not here, most likely first run of program
	if( !file ) {
		return;
	}

	while( std::getline( file, line ) ) {
		DefinitionLine def;

		if( line.empty( ) || line.front( ) == ' ' ) {
			def.blank = true;
		}

		if( !def.blank ) {
			// If a comment
			if( line.substr( 0, 2 ) == "//" ) {
				def.comment = true;

				// Parse whole line and put in name
				def.name = line;
			}

			// See if we have reached the end of file
			if( line.substr( 0, 3 ) == "EOF" ) {
				break;
			}

			if( !def.comment ) {
				// Find space position
				int spacePos = 0;
				for( char c : line ) {
					if( c == ' ' ) {
						break;
					}
					spacePos++;
				}

				// Parse var name
				def.name = line.substr( 0, spacePos );

				// Parse float value
				valBuf = line.substr( spacePos + 1 );
			}

			def.valstr = valBuf;
		}
		// Put in new definition
		definitionLines.push_back( def );
	}
	file.close( );
}

/* 
==============================================
VariableFile::DefinitionLine *VariableFile::getDefinition( std::string_view varname )
==============================================
*/
VariableFile::DefinitionLine *VariableFile::getDefinition( std::string_view varname ) {
	  //loop through vector
	  for( auto &v : definitionLines ) {
		  //check to see if def line is a comment or a blank
		  if( v.blank || v.comment ) {
			  continue;
		  }
		  //compare variable defintion name to requested name
		  if( v.name == varname ) {
			  //matching definition found - return it
			  return &v;
		  }
	  }
	  return NULL;
}

/* 
==============================================
void VariableFile::saveFile

  TODO check to see that definition lines are memory managed properly
==============================================
*/
void VariableFile::saveFile( std::string_view filename, Variables *vars ) {
	std::ofstream file( filename.data() );
		
	//get master name list
	std::list< std::string* > masterNameList;
	vars->getNameList( &masterNameList );

	//go through master list of iterator
	std::list< std::string* >::iterator i;
	for( i = masterNameList.begin(); i != masterNameList.end(); i++ ) {
		std::string *currVarName = ( std::string* )*i;
		
		if( vars->getSaveVarToFile( *currVarName ) ) {
			//try to find def
			DefinitionLine *def = getDefinition( *(*i) );
			if( def == NULL ) {
				//def not found so create a new one
				DefinitionLine newdef;
				
				//copy current variable into new definition
				newdef.name = *currVarName;
				
				//convert value to string
				newdef.valstr = vars->getValueString( *currVarName );

				//add to definition list
				this->definitionLines.push_back( newdef );
			}
			//def was found simply change it
			else {
				def->valstr = vars->getValueString( *currVarName );
			}
		}
	}
	
	//all vars with saveToFile status have now been added to line list 
	//now write to file
	for( auto v : this->definitionLines ) {
		if( v.blank ) {
			file << " \n";
			continue;
		}
		else if( v.comment ) {
			file << v.name << "\n";
		}
		else { //normal instance
			file << v.name << " " << v.valstr << "\n";
		}
	}
	//TODO get rid of this
	file.write( "EOF", 3 );
	file.close();
}
