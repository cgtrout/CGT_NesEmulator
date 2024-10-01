#if !defined( MESSAGESYSTEM_INC )
#define MESSAGESYSTEM_INC
#include <list>
#include "StringToNumber.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

///////////////////////////////////
//  systems
///////////////////////////////////
#include "NesCpuCore.h"
#include "NesDebugger.h"

///////////////////////////////////

/*
=================================================================
=================================================================
MessageCommand Class

  a message command is one command that is contained within a 
  category
=================================================================
=================================================================
*/
class MessageCommand {
public:
	MessageCommand( ) {}
	virtual ~MessageCommand( ) {}
	
	virtual void run( std::string args, bool echo ) {}
	std::string getStringForm( ) {return stringForm;}
protected:
	std::string stringForm;
	std::string paramater;
};

/*
=================================================================
=================================================================
MessageVariable : public MessageCommand Class

  allows variables to be set from the console
=================================================================
=================================================================
*/

enum {
		MV_FLOAT,
		MV_INT
};

class MessageVariable : public MessageCommand {
public:
	MessageVariable( ):intvalue( 0 ),floatvalue( 0.0f ),strvalue( "0" ) {
		
	}
	virtual ~MessageVariable( ) {}

	virtual void run( std::string args, bool echo );

	int getIntValue( ) {return intvalue;}
	float getFloatValue( ) {return floatvalue;}

	void setValue( std::string value ) {run( value, false );}
	
protected:
	int type;

	int intvalue;
	float floatvalue;
	std::string strvalue;
};

/*
=================================================================
=================================================================
MessageCategory Class

  a message category contains a number of message commands
=================================================================
=================================================================
*/
class MessageCategory {
public:
	MessageCategory( ) {
		
	}

	virtual ~MessageCategory( ) {}

	void addListCommand( ) {
		//create a 'list' command
		listCommand.setListOfCommands( commands );
		commands.push_back( &listCommand );
	}

	std::string getStringForm( ) {return stringForm;}

	std::list< MessageCommand* > getCommands( ) {return commands;}
protected:
	std::list< MessageCommand* > commands;
	std::string stringForm;

	class ListCommand : public MessageCommand {
	public:	
		ListCommand( ) {
			stringForm = "list";
		}

		void setListOfCommands( std::list< MessageCommand* > listCommands ) {
			lccommands = listCommands;
		}

		void run( std::string args, bool echo );
	private:
		std::list< MessageCommand* > lccommands;
	}listCommand;
};


/*
=================================================================
=================================================================
MessageSystem Class

  controls the categories and their corresponding commands
=================================================================
=================================================================
*/
class MessageSystem {
public:
	MessageSystem( ) {
		history = "\n";
		categories.push_back( &mCatRoot );
		categories.push_back( &mCatNes );

	}
	virtual ~MessageSystem( ) {}

	//runs command "line" given as string
	void runCommand( std::string line, bool echo = true );
	
	//returns history string, which is a the history of commands entered into the system
	std::string getHistory( ) {return history;}

	//adds string to history string
	void printString( std::string ps ) {
		history += ps.c_str( );
		history += "\n";
	}

	
	std::list< MessageCategory* > categories;
	void parseLine( std::string line );
	std::string history;

	//contains the currently running command's arguments
	std::string category;
	std::string command;
	std::string args;
	
	/*
	=================================================================
	=================================================================
	Root Category
	=================================================================
	=================================================================
	*/
	class MCatRoot : public MessageCategory {
	public:
		MCatRoot( ) {
			commands.push_back( &mComTestCommand );
			commands.push_back( &mComPrint );
			commands.push_back( &mVarTestVar );
			commands.push_back( &mComQuit );
			stringForm = "root";

			addListCommand( );
		}
		virtual ~MCatRoot( ) {}
//	protected:
		/*
		==============================================
		MVarTestVar
			tests variables
		==============================================
		*/
		class MVarTestVar : public MessageVariable {
		public:
			MVarTestVar( ) {
				stringForm = "testvar";
				type = MV_INT;
			}
		}mVarTestVar;
		/*
		==============================================
		MComTestCommand
			tests command
		==============================================
		*/
		class MComTestCommand : public MessageCommand {
		public:
			MComTestCommand( ) {
				stringForm = "testcommand";
			}
			virtual ~MComTestCommand( ) {}

			void run( std::string args, bool echo );
		}mComTestCommand;
		/*
		==============================================
		Print
			prints arg
		==============================================
		*/
		class MComPrint : public MessageCommand {
		public:
			MComPrint( ) {
				stringForm = "print";
			}
			virtual ~MComPrint( ) {}

			void run( std::string args, bool echo );
		}mComPrint;
		/*
		==============================================
		Quit
			
		==============================================
		*/
		class MComQuit : public MessageCommand {
		public:
			MComQuit( ) {
				stringForm = "quit";
			}
			virtual ~MComQuit( ) {}

			void run( std::string args, bool echo );
		}mComQuit;
	}mCatRoot;

	/*
	=================================================================
	=================================================================
	Nes Category
	=================================================================
	=================================================================
	*/
	class MCatNes : public MessageCategory {
	public:
		MCatNes( ) {
			stringForm = "nes";
			
            commands.push_back( &mComDebug );
            commands.push_back( &mComLoad );
            commands.push_back( &mVarPatternTableX );
            commands.push_back( &mVarPatternTableY );
            commands.push_back( &mVarDrawPatternTable );
            
			addListCommand( );
		}
//	protected:
		/*
		==============================================
		load
			loads file
		==============================================
		*/
		class MComLoad : public MessageCommand {
		public:
			MComLoad( ) {
				stringForm = "load";
			}
			virtual ~MComLoad( ) {}

			void run( std::string args, bool echo );
		}mComLoad;
		
		/*
		==============================================
		debug
			loads debugger
		==============================================
		*/
		class MComDebug : public MessageCommand {
		public:
			MComDebug( ) {
				stringForm = "debug";
			}
			virtual ~MComDebug( ) {}

			void run( std::string args, bool echo );
		}mComDebug;
		
		/*
		==============================================
		PatternTableX
			x position for rendered patterntable
		==============================================
		*/
		class MVarPatternTableX : public MessageVariable {
		public:
			MVarPatternTableX( ) {
				stringForm = "patterntablex";
				type = MV_INT;
				
				intvalue = 0;
			}
		}mVarPatternTableX;
        /*
		==============================================
		PatternTableY
			y position for rendered patterntable
		==============================================
		*/
		class MVarPatternTableY : public MessageVariable {
		public:
			MVarPatternTableY( ) {
				stringForm = "patterntabley";
				type = MV_INT;
				
				intvalue = 0;
			}
		}mVarPatternTableY;
		/*
		==============================================
		DrawPatternTable
			set to 1 to draw pattern table
		==============================================
		*/
		class MVarDrawPatternTable : public MessageVariable {
		public:
			MVarDrawPatternTable( ) {
				stringForm = "drawpatterntable";
				type = MV_INT;
				
				intvalue = 0;
			}
		}mVarDrawPatternTable;
	}mCatNes;
};

#endif // !defined( MESSAGESYSTEM_INC )
