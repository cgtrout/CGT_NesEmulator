// GameMain.h: interface for the GameMain class.
//
//////////////////////////////////////////////////////////////////////

#if !defined( AFX_GAMEMAIN_H__7ABE258F_5B1F_4833_B5F6_0A06D5FBEF42__INCLUDED_ )
#define AFX_GAMEMAIN_H__7ABE258F_5B1F_4833_B5F6_0A06D5FBEF42__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#pragma warning( disable : 4786 ) 
#pragma warning( disable : 4503 ) 
#endif // _MSC_VER > 1000

#include "GLRenderer.h"
//#include "Timer.h"
#include "Input.h"
#include "Log.h"
//#include "Vector.h"

#include "GUIConsole.h"

//#include "MessageSystem.h"

extern HWND hWnd;
extern Input *input;
extern Timer *timer;
extern CLog *_log;
//extern MessageSystem *ms;


/*
================================================================
================================================================
GameMain

  Front end class - responsible for updating appropriate classes
                    each frame
================================================================
================================================================
*/
class GameMain {
public:
	
	//sets up and starts the game
	void startGame( );
	
	//runs a frame of the game world
	void runFrame( );
	
	GameMain( );
	virtual ~GameMain( );

	//sets dormant state for a graphic object
	void setDormantState( int id, bool val );

	//does the user want to quit?
	inline bool getWantsQuit( ) {return wantsQuit;}

	void setWantsQuit( bool val ) {wantsQuit = true;}

	//inline Input *getInput( ) {return input;}
	//inline void setInput( Input *input ) {this->input = input;}
	inline Timer *getTimer( ) {return timer;}

	inline float getElapsedTime( ) {return elapsedTime;}
	inline float getCurrTime( ) {return currTime;}

	//inline CLog *getLog( ) {return &log->Get( );}

	//prints average fps to client log
	void printAvgFps( ) {
		float avgfps = 1/( totalElapsedTime / frameNumber );
		_log->Write( "Average fps = %f", avgfps );
	}

	//updates all graphics objects so they are current
	void graphicUpdate( );

	Renderer *getRenderer( ) {return renderer;}
	GUISystem::GUI *getGui( ) {return gui;}

private:
	Renderer *renderer;
	GUISystem::GUI *gui;
	GUIConsole *gc;
	
	bool wantsQuit;
	float elapsedTime;
	float currTime;
	
	float totalElapsedTime;
	float frameNumber;
};

extern GameMain *gm;

#endif // !defined( AFX_GAMEMAIN_H__7ABE258F_5B1F_4833_B5F6_0A06D5FBEF42__INCLUDED_ )
