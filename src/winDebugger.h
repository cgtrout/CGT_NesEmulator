#if !defined( WinDebugger_INCLUDED )
#define WinDebugger_INCLUDED

#include < windows.h >

#include "CGTSingleton.h"
using namespace CGTSingleton;

#include "nesCpuCore.h"

typedef unsigned short uword ;
namespace NesEmulator {
	/*
	==============================================
	class WinDebugger

	this controls a win32 based debugger window
	==============================================
	*/
	class WinDebugger {
	public:
		//windows callback functions
		static BOOL CALLBACK debugDlgProc( HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam );
		static LONG FAR CALLBACK List3Proc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
		static LONG FAR CALLBACK StepButtonProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
		
		//HWND get/set
		void setDebugWnd( HWND val ) {debugWnd = val;}
		HWND getDebugWnd() {return debugWnd;}

		void showWindow();
		void hideWindow();
		void initialize();

		//loads text into disassembly window
		void loadDisassemblyWindow( char *text );
		
		//fill/update the debug window
		void fillWindow();

		//this will load memory dump values from nesMemory
		static void loadMemoryDump();

		int getCurrentSelectedAddress();
		static void selectDissasemblerLine( int op );
		
		const int numDebugLines;

		bool isOpen() {return open;}
		void onEnter();

		WinDebugger();
		~WinDebugger() {}

	protected:

	private:
		
		static HWND debugWnd;
		static FARPROC fnOldList3Proc;

		static uword parseAddressFromText( char *text );
		static int parseAddress( int controlId );
		void fillWatch( int watchToGet, int watchToFill );
		static void gotoButtonClicked();
		static int listPos;
		static bool open;
	};

	static WinDebugger *winDebugger = NULL;
}
#endif //WinDebugger_INCLUDED
