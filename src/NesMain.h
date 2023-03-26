#pragma once

typedef unsigned char ubyte;
typedef unsigned short uword;
typedef short word;

#include "CGTSingleton.h"
using namespace CGTSingleton;

#ifndef LIGHT_BUILD
	#include "NesDebugger.h"
#endif

#include "NesController.h"
#include "NesPpu.h"
#include "NesSound.h"

//forward declare systemMain
//we pass this to NesMain so that it can pass the input system to NesController
//Ideally need to eliminate this double coupling
namespace FrontEnd {
	class SystemMain;
}

namespace NesEmulator {

	class NesDebugger;

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
		NesMain( FrontEnd::SystemMain* );
		~NesMain() { }
		
		//update emulator for this frame
		void runFrame();

		void ApuCpuSync( );
		void PpuCpuSync( );
		
		//reset variables in this class
		void reset();
	
		PpuClockCycles CyclesPerFrame;
		NesEmulator::NesFile				nesFile;

		NesEmulator::NesCpu					nesCpu;
		
		NesEmulator::NesControllerSystem	controllerSystem;
		NesEmulator::NesMemory				nesMemory;
		
		NesEmulator::NesPPU					nesPpu;
		NesEmulator::NesEmulatorFlagSystem  emulatorFlags;

		NesApu::NesSound					nesApu;

	#ifndef LIGHT_BUILD
		NesEmulator::NesDebugger			nesDebugger;
	#endif
		
		FrontEnd::SystemMain* systemMain;

		void setState ( States s ) { state = s; }
		States getState() { return state; }
	  private:
		PpuClockCycles masterClock;
		PpuClockCycles cpuOverflow;
		float frameTime;	
		int frameNum;

		States state;
	};
}
