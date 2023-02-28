#if !defined( commandhandler__H )
#define commandhandler__H
#include "CGTSingleton.h"
using namespace CGTSingleton;
namespace Console {
	//#include "Console.h"

	struct ConsoleCommand;
	class CommandHandlerSystem : public Singleton< CommandHandlerSystem > {
	public:
		friend Singleton< CommandHandlerSystem >;
		ConsoleCommand *getCommands();

		void quit( const char *param );
		void loadNesFile( const char *param );
		
		//cpu trace stuff
  #ifndef LIGHT_BUILD
		void printTraceLog( const char *param );
		void printAsm( const char *param );
		void startTrace( const char *param );
		void stopTrace( const char *param );
  #endif
		//key bind command
		void bindKey( const char *param );

		//resets cpu
		void reset( const char *param );

		void help( const char *param );
		
	private:
		CommandHandlerSystem();
		
		void printBindKeyUsage( const char * );
		void printHelpUsage ( const char * );
	};

	typedef void ( CommandHandlerSystem::*CCommandFnptr )( const char * );

	struct ConsoleCommand {
		std::string name;
		CCommandFnptr handler;	//function pointer to command handler
		std::string description;
	} ;

}//namespace
#endif