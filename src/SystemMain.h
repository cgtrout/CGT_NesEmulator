// SystemMain.h: interface for the SystemMain class.
//
//////////////////////////////////////////////////////////////////////

#if !defined( AFX_SystemMain_H__7ABE258F_5B1F_4833_B5F6_0A06D5FBEF42__INCLUDED_ )
#define AFX_SystemMain_H__7ABE258F_5B1F_4833_B5F6_0A06D5FBEF42__INCLUDED_

//TODO investigate these supressed warnings
#if _MSC_VER > 1000
#pragma once
#pragma warning( disable : 4786 ) 
#pragma warning( disable : 4503 ) 
#endif // _MSC_VER > 1000

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

#endif // !defined( AFX_SystemMain_H__7ABE258F_5B1F_4833_B5F6_0A06D5FBEF42__INCLUDED_ )
