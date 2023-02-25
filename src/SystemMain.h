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

#include "Console.h"
#include "GLRenderer.h"
#include "Input.h"
#include "NesMain.h"

#include "GUIConsole.h"
#include "GUIFPSCounter.h"
#include "GuiTimeProfiler.h"

#include "FPSTimer.h"
#include "TimeProfiler.h"

#include "CGTSingleton.h"

//#ifdef WIN32
 #include "SoundSystem.h"
//#endif

using namespace CGTSingleton;

extern HWND hWnd;

namespace FrontEnd {
	
	/*
	================================================================
	================================================================
	SystemMain

	  Front end class - responsible for updating appropriate classes
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

		Console::ConsoleSystem	consoleSystem;
		
		GUISystem::GUI			gui;
		GUISystem::GUIConsole	gc;
		
		NesEmulator::NesMain	nesMain;
		
		Render::Renderer		renderer;
		
		GUISystem::GUIFPSCounter frameCounter;
		FPSTimer fpsTimer;
		
		GUISystem::GuiTimeProfiler guiTimeProfiler;
		TimeProfiler timeProfiler;

		Sound::SoundSystem *soundSystem;
		
	  private:
		
		SystemMain();
		virtual ~SystemMain();

		bool	wantsQuit;
	};
}


#endif // !defined( AFX_SystemMain_H__7ABE258F_5B1F_4833_B5F6_0A06D5FBEF42__INCLUDED_ )
