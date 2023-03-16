#if !defined( NesEmulatorFlags_INCLUDED )
#define NesEmulatorFlags_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <deque>

#include "CGTSingleton.h"
#include "typedefs.h"
using namespace CGTSingleton;

namespace NesEmulator {
	/*
	=================================================================
	=================================================================
	NesEmulatorFlagSystem Class

	 these are global flags that are used to alert 
	 various modules to kick into action
	=================================================================
	=================================================================
	*/
	
	enum FlagTypes {
		FT_UpdatePPU,		//used to tell cpu that the ppu needs to be updated
		FT_UpdateAPU,
		FT_VINT,			//start of vint has been reached
		FT_SingleStep,		//have done a singlestep
		FT_SkipAddTime,		//tells the cpu to not factor the time of the current instruction
		FT_IrqRequest		//an irq has been raised
	};

	class NesEmulatorFlagSystem  {
	public:
		NesEmulatorFlagSystem();
		
		//how many flags do we have in the system
		//be sure to change this as FlagTypes are added
		static const int NUMFLAGS = 6;

		//do we have any flags set in the system
		bool flagsPresent();
		
		//set flag, along with the time it was set ( in relation to last VINT )
		void setFlag( FlagTypes, PpuClockCycles c = 0 );
		void unsetFlag( FlagTypes );
		bool isFlagSet( FlagTypes );
		
		//get the clockcycle that flag was set on
		PpuClockCycles getFlagSetTime( FlagTypes );

		//clear all of the flags
		void clearFlags();

		//PpuClockCycles getFlagTime();
		
	private:	
		struct Flag {
			bool flagSet;
			PpuClockCycles time;		//time clockcycle was set in relation to
										//vint
		};
		Flag flags[ NUMFLAGS ];
	};
	
}

#endif //NesEmulatorFlags_INCLUDED