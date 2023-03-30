// SystemMain.h: interface for the SystemMain class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "CGTSingleton.h"
#include "GUI.h"
#include "GUIConsole.h"
#include "TimeProfiler.h"
#include "NesMain.h"
#include "GLRenderer.h"

using namespace CGTSingleton;

namespace FrontEnd {
	
	/*
	================================================================
	================================================================
	SystemMain

	  Front end class - responsible for updating all other objects
						each frame

						this system coordinates the actions between
						the emulator and the front end 
	================================================================
	================================================================
	*/
	class SystemMain : public Singleton< SystemMain > {
	 friend class fpsTimer;
	  public:
		  
		friend Singleton< SystemMain >;
		
		void initialize( );

		void loadNesFile( std::string_view fileName );

		//sets up and starts the class
		void start();
		
		//runs one frame
		void runFrame();

		//sets dormant state for a graphic object
		void setDormantState( int id, bool val );

		//does the user want to quit?
		bool quitRequestSubmitted() { return wantsQuit; }

		//send quit request
		void quitRequest() { wantsQuit = true; }

		//updates all graphics objects
		void graphicUpdate();

		Input					input;

		Console::ConsoleSystem	consoleSystem;
		
		GUISystem::GUI			gui;
		GUISystem::GUIConsole	guiConsole;
		
		NesEmulator::NesMain	nesMain;
		
		Render::Renderer		renderer;
		
		TimeProfiler timeProfiler;

		//TestingSystem
		//This allows us to set a test directory
		//and then load a rom at a time to test multiple
		//roms easily
		struct TestingSystem {
			bool inTestMode = false;
			std::vector<std::string> files;
			std::string directory;
			std::vector<std::string>::iterator iter;

			//takes in directory name and builds test list 
			//of roms that reside in that dir
			void buildDirVector( std::string_view dirName );
		}testingSystem;

		//load next test from the testing system
		void loadNextTest( );

	  private:
		
		SystemMain();
		virtual ~SystemMain();

		bool	wantsQuit;
	};
}

