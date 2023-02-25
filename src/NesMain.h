#if !defined( NesMain_INCLUDED )
#define NesMain_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef unsigned char ubyte;
typedef unsigned short uword;
typedef short word;

#include "CGTSingleton.h"
using namespace CGTSingleton;

#ifndef LIGHT_BUILD
	#include "NesDebugger.h"
#endif

//#include "NesFileLoader.h"
#include "NesController.h"
#include "NesSound.h"
#include "NesCpuCore.h"

namespace NesEmulator {
	/*
	=================================================================
	=================================================================
	NesMain Class

	 This class controls the operation of the emulator
	=================================================================
	=================================================================
	*/

	//possible run states of the emulator
	enum States { Emulating, WaitingForFile };

	class NesMain {
	  public:
		NesMain();
		~NesMain() { }
		
		//update emulator for this frame
		void update();
		
		//reset variables in this class
		void reset();
	
		PpuClockCycles CyclesPerFrame;
		NesEmulator::NesFile				nesFile;

		NesEmulator::NesCpu					nesCpu;
		
		NesEmulator::NesControllerSystem	controllerSystem;
		NesEmulator::NesMemory				nesMemory;
		
		PpuSystem::NesPPU					nesPpu;
		NesEmulator::NesEmulatorFlagSystem  emulatorFlags;

		NesApu::NesSound					nesApu;

	#ifndef LIGHT_BUILD
		NesEmulator::NesDebugger			nesDebugger;
	#endif
		
		void setState ( States s ) { state = s; }
		States getState() { return state; }
	  private:
		//float elapsedTime;
		PpuClockCycles masterClock;
		PpuClockCycles cpuOverflow;
		float frameTime;	
		int frameNum;

		States state;
	};
}
#endif //NesMain_INCLUDED
