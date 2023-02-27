#include "precompiled.h"

using namespace CgtString;

#include "ConsoleCommands.h"
using namespace Console;

#include "StringToNumber.h"

#include < sstream >

//string warning
#if _MSC_VER > 1000
#pragma warning ( disable : 4996 )
#endif

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
		throw new ConsoleSystemException( "Variables::addVariable", ss.str().c_str(), true );
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
	using namespace CgtString;

	//go through entire varList
	typename std::list < ConsoleVariable< T >* >::iterator i;
	for( i = varList->begin(); i != varList->end(); i++ ) {
		ConsoleVariable< T > *t = *i;
		if( toLower( name ) == toLower( *t->getName() ) ) {
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
	ConsoleVariable< int > *i	 = getIntVariable( varName );
	ConsoleVariable< float > *f  = getFloatVariable( varName );
	ConsoleVariable< bool > *b   = getBoolVariable( varName );
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
	ConsoleVariable< int > *i	 = getIntVariable( varName );
	ConsoleVariable< float > *f  = getFloatVariable( varName );
	ConsoleVariable< bool > *b   = getBoolVariable( varName );
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
	ConsoleVariable< int > *i	 = getIntVariable( varName );
	ConsoleVariable< float > *f  = getFloatVariable( varName );
	ConsoleVariable< bool > *b   = getBoolVariable( varName );
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
		stringVector->push_back( (*i)->getName() );
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
	ConsoleCommand *commands = commandHandlerSystem->getCommands();

	//add commands to list
	while( commands->name != "END_OF_LIST" ) {
		addCommand( commands++ );
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
	CgtString::StringTokenizer st;
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
void ConsoleSystem::executeRequest( std::string_view command, std::string_view value, bool echo ) {
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
	printMessage( """%s %s"" was not recognized", command, value );
}


/* 
==============================================
bool ConsoleSystem::findAndRunVariable( std::string *varName, std::string *value, bool run )
==============================================
*/
bool ConsoleSystem::findAndRunVariable( std::string_view varName, std::string_view value, bool run ) {
    //go through variables until varName is found
	ConsoleVariable< int >    *i = variables.getIntVariable( varName );
	ConsoleVariable< float >  *f = variables.getFloatVariable( varName );
	ConsoleVariable< bool >   *b = variables.getBoolVariable( varName );
	ConsoleVariable< std::string > *s = variables.getStringVariable( varName );
	
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
	std::list< ConsoleCommand * >::iterator iter;
	for( iter = commands.begin(); iter != commands.end(); iter++ ) {
		curr = ( ConsoleCommand* )( *iter );

		if( CgtString::strcasecmp( curr->name.c_str(), commandName ) ) {
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
	//runs handler function using a member function pointer
	( commandHandlerSystem->*command->handler )( param.data() );
}

/* 
==============================================
ConsoleSystem::getCommandDescription
==============================================
*/
std::string ConsoleSystem::getCommandDescription( std::string_view commandName ) {
	std::list< ConsoleCommand* >::iterator i;
	for( i = commands.begin(); i != commands.end(); i++ ) {
		ConsoleCommand *c = ( ConsoleCommand* )( *i );
		if( toLower( commandName ) == toLower( c->name ) ) {
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
void ConsoleSystem::printMessage( const char *message, ... ) {
    va_list	argptr;
    char buf[ MaxLineSize*2 ];
	char tbuf[ MaxLineSize ];
	
	va_start ( argptr,message );
	vsprintf ( buf,message,argptr );
	va_end ( argptr );

	//chop up line if greater than maximum allowable length
	int size = strlen( buf );
	char *ptr;
	ptr = &buf[ 0 ];
	while( size > 0 ) {
		if( size > MaxLineSize ) {
			strncpy( tbuf, ptr, MaxLineSize-1 );
			tbuf[ MaxLineSize-1 ] = '\0';
			ptr = &buf[ MaxLineSize-1 ];
			size -= MaxLineSize - 1;
		}
		else {
			strncpy( tbuf, ptr, size );
			tbuf[ size ] = '\0';
			size = 0;
		}	
		history.writeLine( tbuf );
	}
}

/* 
==============================================
char *ConsoleSystem::getHistoryLines( int numLinesToGet, int pos )
  get the last 'numLines' number of lines printed to console hi
==============================================
*/
char *ConsoleSystem::getHistoryLines( int numLinesToGet, int pos ) {
    static int lastPos = 0;

	if( history.numLines < numLinesToGet - pos ) {
		return history.getBuffer( 0, history.numLines );
	}
	if( history.numLines == 0 )
	 {
		return NULL;
	}
	int startLine = history.numLines - numLinesToGet - pos;
	return history.getBuffer( startLine, startLine + numLinesToGet );
}


/* 
==============================================
char *ConsoleSystem::History::getBuffer( int startline, int endline )
==============================================
*/
char *ConsoleSystem::History::getBuffer( int startline, int endline ) {
    char linebuf[ MaxLineSize ];
    int pos = 0;
	int lineLength = 0;
    
	//incase we're shutting down
	if(returnBuffer == NULL) {
		return NULL;
	}

    //clear buffer
    for( int x = 0; x < MaxReturnBufferSize; x++ ) {
        returnBuffer[ x ] = 0;
    }
    for( ; startline < endline; startline++ ) {
        if( lineLength >= MaxReturnBufferSize ) {
            //TODO print message
            return NULL;
        }
        strcpy( &linebuf[ 0 ], getLine( startline ) );
		
        lineLength = strlen( linebuf );
        strcat( returnBuffer, linebuf );
		strcat( returnBuffer, "\n" );
        pos += lineLength;
    }
	return returnBuffer;
}

/* 
==============================================
void ConsoleSystem::History::writeLine( char *string )
==============================================
*/
void ConsoleSystem::History::writeLine( char *string ) {
    //write to current line
    strcpy( lines[ currentLine ].text, string );
    int length = strlen( string );
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
char *ConsoleSystem::History::getLine( int lineToGet )
==============================================
*/
char *ConsoleSystem::History::getLine( int lineToGet ) {
    int firstLinePos, offset, pos;
    
    if( lineToGet >= numLines ) {
        return NULL;
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
char *ConsoleSystem::printMatches( const char *partial )
  used to show possible commands for user when they hit tab with a partial command
  returns match if only one found
  otherwise it returns NULL

TODO return string that matches given partial until the point where the matches no longer match up
==============================================
*/
std::string ConsoleSystem::printMatches( std::string_view partial ) {
	std::list< std::string* > matchList;

	int partialLength = partial.length();
	int matchesFound = 0;
	std::string lastMatch;	//last match found string
	
	if( partialLength == 0 ) {
		return std::string();
	}

	//convert to lowercase
	std::string plcase = CgtString::strtolower( partial );
	
	//go through all commands
	std::list< ConsoleCommand * >::iterator citer;
	ConsoleCommand *ccurr;
	for( citer = commands.begin(); citer != commands.end(); citer++ ) {
		ccurr = ( ConsoleCommand* )( *citer );
		std::string compstr = CgtString::strtolower( ccurr->name.c_str() );
		
		//if current command's name == partial
		if( strncmp( compstr.c_str(), plcase.c_str(), partialLength ) == 0 ) {
			//partial match made so add it to list
			matchList.push_back( &ccurr->name );			
			matchesFound++;
			lastMatch = ccurr->name;
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
		std::string compstr = CgtString::strtolower( cvar->c_str() );
		
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

  TODO return exception?
  TODO test
==============================================
*/
void ConsoleSystem::loadCommandFile( std::string_view filename ) {
	std::ifstream file( filename.data() );

	char line[ 256 ];
	char *ptr;

	if ( !file ) {
		return;
	}
	
	while( !file.eof() ) {
		ptr = &line[0];
		file.getline( ptr, 256 );
				
		//comment line ignore
		if( strncmp( ptr, "//", 2 ) == 0 ) {
			
			continue;
		}
		//pass command to system
		executeRequest( ptr, false );
	}
}

/* 
==============================================
void VariableFile::loadFile( const char *filename )

  TODO change this to use C++ strings/streams?
==============================================
*/
void VariableFile::loadFile( const char *filename ) {
	std::ifstream file( filename );
	char line[ CMAXNAMESIZE ];
	char valBuf[ 12 ];
	char *ptr;

	//check for error.  If file not here, most likely first run of program
	if ( !file ) {
		return;
	}

	while( !file.eof() ) {
		file.getline( line, CMAXNAMESIZE );
		ptr = line;

		DefinitionLine *def = new DefinitionLine();
	
		if( *ptr == 0 || *ptr == ' ' ) {
			def->blank = true;
		}
		
		if( !def->blank ) {
			//if a comment
			if( ptr[ 0 ] == '/' && ptr[ 1 ] == '/' ) {
				ptr = &ptr[ 2 ];
				def->comment = true;

				//parse whole line and put in name
				def->name = line;
			}

			//see if we have reached end of file
			if( strncmp( ptr, "EOF", 3 ) == 0 ) {
				break;
			}

			if( !def->comment ) {
				//find space position 
				int spacePos = 0;
				while( true ) {
					if( ptr == NULL ) {
						break;
					}
					if( *ptr == ' ' ) {
						ptr++;
						break;
					}
					ptr++;
					spacePos++;
				}
				
				//parse var name
				//strncpy( def->name, line, spacePos );
				def->name = line;
				def->name = def->name.substr( 0, spacePos );
				def->name[ spacePos ] = '\0';

				//parse float value
				//ptr = &ptr[ spacePos + 1 ];
				strcpy( valBuf, ptr );
			}

			def->valstr = valBuf;
		}
		//put in new definition
		vars.push_back( def );
	}
	file.close();
}

/* 
==============================================
VariableFile::DefinitionLine *VariableFile::getDefinition( std::string_view varname )
==============================================
*/
VariableFile::DefinitionLine *VariableFile::getDefinition( std::string_view varname ) {
	  //loop through vector
	  for( unsigned int x = 0 ; x < vars.size(); x++ ) {
		  //check to see if def line is a comment or a blank
		  if( vars[ x ]->blank || vars[ x ]->comment ) {
			  continue;
		  }
		  //compare variable defintion name to requested name
		  if( vars[ x ]->name == varname ) {
			  //matching definition found - return it
			  return vars[ x ];
		  }
	  }
	  return NULL;
}

/* 
==============================================
void VariableFile::saveFile( const char *filename, list< ConsoleVariable* > *vars )

  TODO check to see that definition lines are memory managed properly
==============================================
*/
void VariableFile::saveFile( const char *filename, Variables *vars ) {
	std::ofstream file( filename );
		
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
				def = new DefinitionLine();
				
				//copy current variable into new definition
				def->name = *currVarName;
				
				//convert value to string
				def->valstr = vars->getValueString( *currVarName );

				//add to definition list
				this->vars.push_back( def );
			}
			//def was found simply change it
			else {
				def->valstr = vars->getValueString( *currVarName );
			}
		}
	}
	
	//all vars with saveToFile status have now been added to line list 
	//now write to file
	for( unsigned int x = 0; x < this->vars.size(); x++ ) {
		if( this->vars[ x ]->blank ) {
			file << " \n";
			continue;
		}
		else if( this->vars[ x ]->comment ) {
			file << this->vars[ x ]->name << "\n";
		}
		else { //normal instance
			file << this->vars[ x ]->name << " " << this->vars[ x ]->valstr << "\n";
		}
	}
	//TODO get rid of this
	file.write( "EOF", 3 );
	file.close();
}
