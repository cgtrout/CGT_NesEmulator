#if !defined( NESDEBUGGER_INC )
#define NESDEBUGGER_INC

#define MAX_BREAKPOINTS 256

#include < string >

#include "CGTSingleton.h"
using namespace CGTSingleton;

#include "WinDebugger.h"

typedef unsigned short uword;
namespace NesEmulator {
	
#ifndef LIGHT_BUILD 
	class NesDebugger {
	public:
		void initialize();
	    
		void loadWindowText();
		void updateDebugger();
		
		//turn on single step mode
		void setToSingleStepMode( uword address );
		
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
		void singleStepRequest() {doSingleStep = true;}

		bool isBreakPointAt( uword address );
		
		//returns true if breakpoint was added
		bool addBreakPoint( uword address );

		bool removeBreakPoint( uword address );
		
		bool isOpen();
		void onEnter();

		//what is the location of the previous instruction to the instruction at "address"
		uword findPreviousInstructionLocation( uword address );
		uword findNextInstructionLocation( uword address );

		void setRenderPos( uword val );
		void setSelectedAddress( uword address ) {selectedPos = address;}
		uword getSelectedAddress() {return selectedPos;}
		void selectDissasemblerLine( int );

		char *buildDebugLine( uword address, const opcodeLookUpTableEntry *l, ubyte opcode, ubyte byte1val, ubyte byte2val );
			
		NesDebugger();
		~NesDebugger();
		
		WinDebugger winDebugger;
	private:
		std::string buildOutputString( uword startAddress, const int length );
		bool instructionIsPointingTo( uword opAddress, uword knownAddress );
		bool isAPreviousValidInst( uword opAddress );
		
		bool singleStepMode;
		bool doSingleStep;
		bool justInSingleStepMode;

		uword breakPoints[ MAX_BREAKPOINTS ];
		uword numBreakPoints;
		uword renderPos;			//address pos to render from
		uword selectedPos;
	};

}

#endif //LIGHT_BUILD
#endif // !defined( NESDEBUGGER_INC )
