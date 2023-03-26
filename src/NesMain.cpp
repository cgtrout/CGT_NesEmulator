#include "precompiled.h"

//#include "CgtException.h"
#include "Console.h"
#include <SDL_audio.h>
#include "SystemMain.h"

using namespace FrontEnd;
using namespace NesEmulator;

#ifndef LIGHT_BUILD
		
#endif

NesMain::NesMain( FrontEnd::SystemMain* systemMain ) :
	systemMain( systemMain ),
	nesDebugger( this ),
	controllerSystem( &systemMain->input ),
	nesCpu( &systemMain->nesMain ),
	nesFile( &systemMain->nesMain ),
	nesMemory( &systemMain->nesMain ),
	nesPpu( &systemMain->nesMain )
{
	CyclesPerFrame = 89342;//89342;
	state = WaitingForFile;
}
		
void NesMain::runFrame() {
	if( state != Emulating ) {
		return;
	}

	//TODO ntsc / val handling
	
	//keep running this frame?
	bool runFrame = true;

	//update emulator systems for this frame
	while( runFrame ) {

		//_log->Write( "Starting Frame" );

		systemMain->timeProfiler.startSection( "Cpu" );

		nesCpu.addOverflow( cpuOverflow );
		cpuOverflow = 0;
		
		nesCpu.runInstructions();

		//add cpu clockcycles to master clock 
		masterClock = nesCpu.getCC();
				
		//flag was hit if we have reached this point
		//find out what flag we hit
		
		//flag update ppu found
		if( emulatorFlags.isFlagSet( FT_UpdatePPU ) ) {
			//update ppu
			nesPpu.renderBuffer( emulatorFlags.getFlagSetTime( FT_UpdatePPU ) );

			//clear flag
			emulatorFlags.unsetFlag( FT_UpdatePPU );
		}
		
		//start of vint hit?
		if( emulatorFlags.isFlagSet( FT_VINT ) ) {
			//must be done for this frame
			//calculate cpu overflow

			PpuClockCycles vintTime = emulatorFlags.getFlagSetTime( FT_VINT );
			cpuOverflow = nesCpu.getCC() + CpuToPpu( nesPpu.registers.executeNMIonVBlank * 7 ) - vintTime;
			
			//cap master clock so we don't render the ppu past this frame
			//masterClock = vintTime - 1;

			//check to see if ppu has caught up with cpu 
			if( nesPpu.getCC() < vintTime ) {
				//allow ppu to catch up
				nesPpu.renderBuffer( vintTime );
			}

			if( nesApu.getCC() < vintTime ) {
				systemMain->timeProfiler.startSection( "Apu" );
				
				//_log->Write( "Updating APU" );
				nesApu.runTo( vintTime );
				
				systemMain->timeProfiler.startSection( "Cpu" );
			}

			//reset internal clock counters
			nesPpu.resetCC();
			nesCpu.resetCC();
			nesApu.resetCC();
			
			nesPpu.initializeFrame();
			
			masterClock = 0;
			runFrame = false;
		}			
		//if debugging / single stepping exit this loop as well
	#ifndef LIGHT_BUILD	
		if( emulatorFlags.isFlagSet( FT_SingleStep ) ) {
			runFrame = false;
		}
	#endif

		//clear flags
		emulatorFlags.clearFlags();
	}

	//queue sound to sdl buffer
	nesApu.queueSound( );

	frameNum++;
}

void NesMain::ApuCpuSync( ) {
	if( nesApu.getCC( ) < nesCpu.getCC( ) ) {
		nesApu.runTo( nesCpu.getCC( ) );
	}
}
void NesMain::PpuCpuSync( ) {
	if( nesPpu.getCC( ) < nesCpu.getCC( ) ) {
		//specify to update until sprite 0 is found
		//TODO will this work in all cases??
		//if( nesPpu.registers.status.sprite0Time == 0 ) {
		//	nesPpu.renderBuffer( nesCpu.getCC(), PpuSystem::UF_Sprite0 );
		//} else {
		nesPpu.renderBuffer( nesCpu.getCC( ) );
		//}
	}
}

void NesMain::reset() {
	masterClock = 0;
	cpuOverflow = 0;
	frameTime = 0.0f;
	frameNum = 0;

	nesPpu.initializeFrame();
	emulatorFlags.clearFlags();
	nesPpu.reset();
}

