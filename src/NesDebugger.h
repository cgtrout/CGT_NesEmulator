#pragma once

#include <string>

#include "CGTSingleton.h"
using namespace CGTSingleton;

typedef unsigned short uword;
namespace NesEmulator {

	class NesMain;
	
#ifndef LIGHT_BUILD 
	class NesDebugger {
	public:
		void initialize();
	    
		//load debug window with text (disassembled lines)
		void loadWindowText();

		//update debugger - called once per frame
		void updateDebugger();

		//draw debugger using imgui
		void draw( );
		
		//turn on single step mode
		void setToSingleStepMode( uword address, std::string_view message );
		
		//turn off single step mode
		void turnOffSingleStepMode();
		
		//are we in single step mode?
		bool inSingleStepMode() { return singleStepMode;}

		bool isFreshSingleStepMode() {return justInSingleStepMode;}
		
		//do we need to do a single step?
		bool isSingleStepPending() {justInSingleStepMode = false;return doSingleStep;}

		//clear single step request
		void clearSingleStepRequest() {doSingleStep = false;}

		//request a single step
		void singleStepRequest( ) { doSingleStep = true; userMessage.clear( ); }

		bool isBreakPointAt( uword address );
		
		//returns true if breakpoint was added
		bool addBreakPoint( uword address );

		bool removeBreakPoint( uword address );
		
		bool isOpen();

		//on keyboard 'enter' pressed
		void onEnter();

		//what is the location of the previous instruction to the instruction at "address"
		uword findPreviousInstructionLocation( uword address );
		uword findNextInstructionLocation( uword address );

		void setRenderPos( uword val );
		void setSelectedAddress( uword address ) {selectedAddress = address;}
		uword getSelectedAddress() {return selectedAddress;}
		void selectDissasemblerLine( int );

		std::string buildDebugLine( uword address, const opcodeLookUpTableEntry *l, ubyte opcode, ubyte byte1val, ubyte byte2val );
			
		NesDebugger( NesEmulator::NesMain *nesMain );
		~NesDebugger();
		
	private:
		NesMain* nesMain;
		NesMemory* memory;
		
		void buildDissassemblerLines( uword startAddress, const int length );
		bool instructionIsPointingTo( uword opAddress, uword knownAddress );
		bool isAPreviousValidInst( uword opAddress );
		void drawWatchBox( const int index );
		
		bool singleStepMode;
		bool doSingleStep;
		bool justInSingleStepMode;

		std::vector<uword> breakPoints;
		uword renderPos;			//address pos to render from
		uword selectedPos;
		uword selectedAddress;

		bool showDebugWindow;
		bool showMemoryDump;
		
		uword dumpAddress;
		std::string dumpAddressStr;
		int dumpSize;
		int memDumpType;
		std::string loadMemoryDump(  );

		struct DebugLine
		{
			uword address;
			std::string line;

			DebugLine( uword a, std::string l ) : address( a ), line( l ) {}
		};

		std::vector<DebugLine> debugLines;
		int numDebugLines = 24;

		std::array<char[ 5 ], 4> watchStrings;

		std::string userMessage = "";	//allow c++ codebase to pass message to debugger at activation
	};

}

#endif //LIGHT_BUILD
