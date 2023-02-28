#include "Precompiled.h"
#include "WinSoundSystem.h"
#include "Windows.h"

#include <vector>

NesApu::NesSoundBuffer *nesSoundBuffer_w;

std::vector<WAVEHDR> waveHdr;

//callback function
void CALLBACK WinSoundSystem::waveOutProc(  HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD dwParam1, DWORD dwParam2 ) {
	//static float lastTime = 0;
	//static float timeDelta = 0;
	switch( uMsg ) {
		case WM_CLOSE:
		case WOM_DONE:
			//float time = timer->getAbsoluteTime();
			//timeDelta = time - lastTime;
			//lastTime = time;
			//_log->Write( "time delta from last WOM_DONE call = %f",  timeDelta);

			//switching data blocks
			WAVEHDR *waveHdr = reinterpret_cast<WAVEHDR*>(dwParam1);
			//waveHdr->dwBufferLength = 
			
			//put sound data in waveHdr->lpData
			nesSoundBuffer_w->fillExternalBuffer( ( word* )waveHdr->lpData, waveHdr->dwBufferLength / 2 );
			
			waveOutWrite( hwo, waveHdr, sizeof( *waveHdr ) );

			nesSoundBuffer_w->resetBufferPos();
			break;
		//case WOM_OPEN:
		//	break;
	}
}

//handle to waveout
HWAVEOUT phwo;

void WinSoundSystem::initialize() {
	MMRESULT res;

	WAVEFORMATEX waveFormat;

	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	
	//assuming 1 channel for now
	waveFormat.nChannels = 1;
	waveFormat.nSamplesPerSec = getSampleRate();
	
	//assuming 16 for now
	waveFormat.wBitsPerSample = 16; 
	waveFormat.nBlockAlign = ( waveFormat.nChannels * waveFormat.wBitsPerSample ) / 8;
	waveFormat.cbSize = 0;
	
	//open sound device
	res = waveOutOpen( &phwo, WAVE_MAPPER, &waveFormat, (DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION );

	if( res != MMSYSERR_NOERROR ) {
		throw Sound::SoundSystemException( "WinSoundSystem::initialize", "Error initializing sound device" );
	}

	//create wavehdr's
	//TODO this is never deleted
	
	waveHdr = std::vector<WAVEHDR>( this->getNumBuffers( ) );
	for( int x = 0; x < this->getNumBuffers(); x++ ) {
		waveHdr[x].lpData			= (LPSTR)this->getBuffer( x )->get();
		waveHdr[x].dwBufferLength	= this->getBufferSize() * 2;
		waveHdr[x].dwBytesRecorded	= 0;
		waveHdr[x].dwUser			= 0;
		
		if( x == 0) {
			waveHdr[x].dwFlags = WHDR_BEGINLOOP;
		} else if( x == this->getNumBuffers() - 1 ) {
			waveHdr[x].dwFlags = WHDR_ENDLOOP;
		}
		waveHdr[x].dwLoops = 0;
		
		//prepare header
		res = waveOutPrepareHeader( phwo, &waveHdr[x], sizeof( waveHdr[x] ) );
		if( res != MMSYSERR_NOERROR ) {
			throw Sound::SoundSystemException( "WinSoundSystem::initialize", "Error preparing header" );
		}

		//zero out buffer
		word *buf = this->getBuffer( x )->get();
		for( int i = 0; i < this->getBufferSize(); ++i ) {
			buf[ i ] = 0;
		}
	}
	this->setInitialized( );
}

void WinSoundSystem::start() {
	if( !this->isInitialized() ) {
		throw Sound::SoundSystemException( "WinSoundSystem::start", "Attempting to start soundsystem without initializing" );
	}
	//MMSYSERR_INVALHANDLE
	
	//first buffer
	MMRESULT res = waveOutWrite( phwo, &waveHdr[0], sizeof( waveHdr[0] ));
	if( res != MMSYSERR_NOERROR ) goto error;
	
	//second buffer
	res = waveOutWrite( phwo, &waveHdr[1], sizeof( waveHdr[1] ));
	if( res != MMSYSERR_NOERROR ) goto error;
	return;
error:
	throw Sound::SoundSystemException( "WinSoundSystem::start", "Error sending data to device" );
}

void WinSoundSystem::shutDown() {
	if( !this->isInitialized() ) {
		return;
	}

	//close sound device
	MMRESULT res = WAVERR_STILLPLAYING;
	while( res != WAVERR_STILLPLAYING ) {
		res = waveOutClose( phwo );	
	}

	this->setUninitialized();
}
void WinSoundSystem::assignNesSoundBuffer( NesApu::NesSoundBuffer* b ) {
	nesSoundBuffer_w = b;
}