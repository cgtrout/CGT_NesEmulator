#if !defined( console__H )
#define console__H

#include "NesCpuCore.h"

extern NesEmulator::NesCpu *nesCpu;

#include "ConsoleCommands.h"

#include "CGTSingleton.h"
#include <vector>
#include <list>

#include "CGString.h"

using namespace CGTSingleton;
/** 
*** CONSOLE 
***
*** This console behaves in a similar fashion to quakes console
***
*** The implementation was kept fairly simple to allow for easy
*** addition of new console variables and commands
***
**/
namespace Console {
	//maximum length of value_string
	#define CMAXVALUELENGTH     20

	//maximum length of a name in a ConsoleVariable
	#define CMAXNAMESIZE        200

	//maximum length of the description
	//TODO implement descriptions for variables
	#define CMAXDESCRIPTIONSIZE 100
    
	#define SAVE_TO_FILE true
	#define NO_SAVE_TO_FILE false
	
	/*
	================================================================
	================================================================
	Class ConsoleVariable

	  represents a independant console variable.  Can only be
	  a int, float, bool, or std::string

	  TODO add max min system
	================================================================
	================================================================
	*/
	template < class T >	
	class ConsoleVariable {
	public:
		ConsoleVariable( T val, std::string name, std::string desc, bool save ) {
			value = val;
			this->name = name;
			this->description = desc;
			saveToFile = save;
		}
		
		std::string *getName() { return &name; }
		std::string *getDescription() { return &description; }
		bool getSaveToFile() { return saveToFile; }

		std::string toString();
		std::string getValueString();

		T getValue() { return value; }
		void setValue( std::string *val );
		void setValue( T val );

		operator T() const { return value; }
	
	private:
		std::string name;
		std::string description;
		bool  saveToFile;
		T value;
	};

	class Variables;

	/*
	================================================================
	================================================================
	Class VariableFile
	  this holds the variables stored in a physical file
	  this stores and retrieves values from a file for the console
	================================================================
	================================================================
	*/
	class VariableFile {
	  public:
		  struct DefinitionLine {
				DefinitionLine(): comment( false ), blank( false ) {};
				std::string name;
				std::string valstr;
				bool comment;
				bool blank;
		  };
	  
		  //load all file definitions from a file
		  void loadFile( const char * );			
		  
		  //save all file defintions to file
		  void saveFile( const char *, Variables *vars );
		  
		  //returns a definition, if it was in the file 
		  //other wise returns null if varname was not found
		  DefinitionLine *getDefinition( std::string *varname );
	  private:
		  std::vector< DefinitionLine* > vars;
	};

	/*
	================================================================
	================================================================
	Class Variables

	  This holds all of the console variables

	  I could not determine a way to make it more generic so it
	  specializes in handling ints, floats, bools, and strings

	  If more are ever needed it should be easy enough to extend 
	  the system though
	================================================================
	================================================================
	*/
	class Variables {
	  friend class VariableFile;
	  public:
		//finds a variable in consoles variable list
		//returns a pointer to varible if found
		ConsoleVariable< int >    *getIntVariable	( std::string *variable );
		ConsoleVariable< float >  *getFloatVariable ( std::string *variable );
		ConsoleVariable< bool >   *getBoolVariable  ( std::string *variable );
		ConsoleVariable< std::string > *getStringVariable( std::string *variable );
				
		//add variable/command to consolesytem
		void addIntVariable   ( ConsoleVariable < int    > *var );
		void addFloatVariable ( ConsoleVariable < float  > *var );
		void addBoolVariable  ( ConsoleVariable < bool   > *var );
		void addStringVariable( ConsoleVariable < std::string > *var );

		//gets a string representation of a value
		std::string getValueString( std::string *varName );
		std::string getVariableDescription( std::string *varname );
		
		//Finds out if variable "varName" should be saved to
		//file or not.
		//Will not return any kind of error if variable does
		//not exist( it will return a false )
		bool getSaveVarToFile( std::string *varName );

		Variables();
		~Variables();

		// returns a name list of all variables
		void getNameList( std::list< std::string* > *results );
		
	  private:
		//TODO memory management
		std::list< ConsoleVariable< int >*    > intVars;
		std::list< ConsoleVariable< float >*  > floatVars;
		std::list< ConsoleVariable< bool >*   > boolVars;
		std::list< ConsoleVariable< std::string >* > stringVars;

		template< class T >
		void addVariable( ConsoleVariable< T > *variable, std::list< ConsoleVariable< T >* > *varList );
		
		template< class T >
		ConsoleVariable< T > *getVariable( std::string  *name, std::list < ConsoleVariable< T >* > *varList );

		//gets name list from a generic list of ConsoleVariables
		template< class T >
		void getNameList( typename std::list< ConsoleVariable< T >* > *genericVarList, std::list< std::string  * > *stringList );
		
		//main variable file - used for saving / loading 
		//variables to a file
		VariableFile variableFile;
	};

	/*
	================================================================
	================================================================
	Class ConsoleSystem 

	  this is the main console system..  Handles both ConsoleCommands
	  and ConsoleVariables.
	================================================================
	================================================================
	*/
	class ConsoleSystem {
	public:
		ConsoleSystem();
		~ConsoleSystem();
		
		void initialize();
		
		//adds a command to console system
		void addCommand ( ConsoleCommand *command );	
		
		//executes a command
		//"echo"	if true, prints out command request to console
		void executeRequest( std::string  *command, bool echo );
		
		//executes a command
		//"command" represents the command to execute
		//"value"	represents the value / or param
		//"echo"	if true, prints out command request to console
		void executeRequest( std::string  *command, std::string  *value, bool echo = true );		
	    
		//get the last 'numLines' number of lines printed to console history starting at 'pos'
		char *getHistoryLines( int numLines, int pos );
		
		//returns the number of lines in the history buffer
		int sizeOfHistory() {return history.numLines;}

		//prints out a message to the console screen
		void printMessage( const char *message, ... );
		
		//runs command given with params
		void runCommand  ( ConsoleCommand  *command, std::string  *param );

		//finds a command and returns its description
		std::string  getCommandDescription( std::string  *commandName );
		
		//used to show possible commands for user when they hit tab with a partial command
		//returns match if only one found
		//otherwise it returns NULL
		std::string  printMatches( std::string  *partial );

		//get previous line from history buffer
		std::string  getPreviousRequest();
		
		//get next line from history buffer
		std::string getNextRequest();

		//loads and executes a file containing commands
		void loadCommandFile( std::string  filename );

		//this class contains all of the variables of the console
		//system
		Variables variables;
		
	private:
		//list of console commands
		std::list< ConsoleCommand* >  commands;

		//history of commands previously entered
		std::list< std::string > prevRequests;
		std::list< std::string >::iterator requestIter;
		unsigned int requestPos;
	    
		//finds a variable - returns true if it exists
		//set run to true to execute/change variable
		bool findAndRunVariable ( std::string  *varName, std::string  *value, bool run = true );
	    
		//finds and possibly runs a command 
		//set run to true to execute the variable
		bool findAndRunCommand ( std::string  *commandName, std::string  *param, bool run = true );

		//History class - container with lines of history for console
		//TODO this is fairly poorly designed and should be 
		//replaced.  Uses too much crusty c string handling
		static const int MaxLines				= 100;
		static const int MaxLineSize			= 63;
		static const int MaxReturnBufferSize	= 1000;
		
		class History {
		public:
			//TODO use std structure for htis
			struct Line {
				char text[ MaxLineSize ];
			}lines[ MaxLines ];   //cyclic array of lines
	        
			//number of lines in buffer
			int numLines;
			int currentLine;
			int firstLinePos;
	        
			//writes a line to console history
			void writeLine( char *string );
	        
			//gets a line from lines array
			char *getLine( int lineToGet );
	        
			//gets a string with lines starting at 'startline', ending at 'endline'
			char *getBuffer( int startline, int endline );
	        
			char returnBuffer[ MaxReturnBufferSize ];
  
			History():numLines( 0 ), currentLine( 0 ) {}
		} history;
		
		//main system used for handling console commands
		CommandHandlerSystem *commandHandlerSystem;
	};


	class ConsoleSystemException : public CgtException {
	public:
		ConsoleSystemException( const char *h, const char *m, bool s = true) { ::CgtException( h, m, s );	}
	private:
		ConsoleSystemException();
	};
}

#endif //console