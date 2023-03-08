#include "Precompiled.h"
#include "STLSoundSystem.h"

#include <vector>

NesApu::NesSoundBuffer *nesSoundBuffer_w = nullptr;


void STLSoundSystem::initialize() {
	
}

void STLSoundSystem::start() {
	
}

void STLSoundSystem::shutDown() {
	if( !this->isInitialized() ) {
		return;
	}
}

