#include "precompiled.h"
#include "NesEmulatorFlagSystem.h"

using namespace NesEmulator;

NesEmulatorFlagSystem::NesEmulatorFlagSystem() {
	clearFlags();
}
void NesEmulatorFlagSystem::clearFlags() {
	for( int x = 0; x < NUMFLAGS; x++ ) {
		flags[ x ].flagSet = false;
		flags[ x ].time = 0;
	}
}

bool NesEmulatorFlagSystem::flagsPresent() {
	//go through array of flags
	for( int x = 0 ; x < NUMFLAGS ; x++ ) {
		//if flag is found to be set then return true
		if( flags[ x ].flagSet ) {
			return true;
		}
	}
	//no flags found
	return false;
}

void NesEmulatorFlagSystem::setFlag( FlagTypes f, PpuClockCycles time ) {
	flags[ f ].flagSet = true;
	flags[ f ].time = time;
}

void NesEmulatorFlagSystem::unsetFlag( FlagTypes f ) {
	flags[ f ].flagSet = false;
	flags[ f ].time = 0;
}

bool NesEmulatorFlagSystem::isFlagSet( FlagTypes f ) {
	return flags[ f ].flagSet;
}

PpuClockCycles NesEmulatorFlagSystem::getFlagSetTime( FlagTypes f ) {
	return flags[ f ].time;
}
