#if !defined( WinSoundSystem_INCLUDED )
#define WinSoundSystem_INCLUDED

#include "SoundSystem.h"

//windows sound system abstraction
class WinSoundSystem : public Sound::SoundSystem {
	void initialize();
	void start();
	void shutDown();
	
	//windows callback procedure
	static void CALLBACK waveOutProc(  HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD dwParam1, DWORD dwParam2 );
	
	void assignNesSoundBuffer( NesApu::NesSoundBuffer* );
};

#endif 