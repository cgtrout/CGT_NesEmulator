#pragma once

#include "CGTSingleton.h"
using namespace CGTSingleton;
namespace Console {
	//#include "Console.h"

	struct ConsoleCommand;
	class CommandHandlerSystem : public Singleton< CommandHandlerSystem > {
	public:
		friend Singleton< CommandHandlerSystem >;
		ConsoleCommand *getCommands();

		void quit( std::string_view param );
		void loadNesFile( std::string_view param );
		
		//cpu trace stuff
  #ifndef LIGHT_BUILD
		void printTraceLog( std::string_view param );
		void printAsm( std::string_view param );
		void startTrace( std::string_view param );
		void stopTrace( std::string_view param );
  #endif
		//key bind command
		void bindKey( std::string_view param );

		//resets cpu
		void reset( std::string_view param );

		void help( std::string_view param );
		
	private:
		CommandHandlerSystem();
		
		void printBindKeyUsage( std::string_view  );
		void printHelpUsage ( std::string_view  );
	};

	typedef void ( CommandHandlerSystem::*CCommandFnptr )( std::string_view );

	struct ConsoleCommand {
		std::string name;
		CCommandFnptr handler;	//function pointer to command handler
		std::string description;
	} ;

}//namespace
