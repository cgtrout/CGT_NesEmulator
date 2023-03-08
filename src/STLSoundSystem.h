#pragma once
#include "SoundSystem.h"

//windows sound system abstraction
class STLSoundSystem : public Sound::SoundSystem {
	void initialize();
	void start();
	void shutDown();
};

