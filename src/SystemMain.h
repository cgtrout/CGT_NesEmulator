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
		
	  private:
		
		SystemMain();
		virtual ~SystemMain();

		bool	wantsQuit;
	};
}

