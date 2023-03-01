#include "Precompiled.h"
#include "WinSoundSystem.h"
#include "Windows.h"

#include <vector>

NesApu::NesSoundBuffer *nesSoundBuffer_w = nullptr;

std::vector<WAVEHDR> waveHdr;

//callback function
void CALLBACK WinSoundSystem::waveOutProc(  HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD dwParam1, DWORD dwParam2 ) {
	switch( uMsg ) {
		case WM_CLOSE:
			break;
		case WOM_DONE:
			//if buffer is null it means sound system is not active
			if ( nesSoundBuffer_w == nullptr ) {
				break;
			}

			//switching data blocks
			WAVEHDR *waveHdr = reinterpret_cast<WAVEHDR*>(dwParam1);
			
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

	//create wavehdr's (wave headers)
	waveHdr = std::vector<WAVEHDR>( this->getNumBuffers( ) );
	for( int x = 0; x < this->getNumBuffers(); x++ ) {
		waveHdr[x].lpData		= (LPSTR)this->getBuffer( x )->get();
		waveHdr[x].dwBufferLength	= this->getBufferSize() * 2;
		waveHdr[x].dwBytesRecorded	= 0;
		waveHdr[x].dwUser		= 0;
		
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

void checkSoundError( MMRESULT res, std::string_view error, std::string_view system ) {
	if ( res != MMSYSERR_NOERROR ) {
		//get reason for error
		char buf[ 256 ];
		auto error_code = GetLastError( );
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, error_code, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
			buf, ( sizeof( buf ) / sizeof( char ) ), NULL );
		std::string str( buf );
		str += ' ';

		throw Sound::SoundSystemException( system.data( ), str + error.data( ) );
	}
}

void WinSoundSystem::shutDown() {
	if( !this->isInitialized() ) {
		return;
	}

	MMRESULT res;

	//assign nullptr to sound buffer to so that callback has a way to notice that sound is being shut down
	this->assignNesSoundBuffer( nullptr );

	_log->Write( "Audio: Starting shutdown" );

	res = waveOutReset( phwo );
	checkSoundError( res, "waveOutReset", "shutdown" );

	res = waveOutClose( phwo );
	checkSoundError( res, "waveOutClose", "shutdown" );

	unprepareHeaders( );

	_log->Write( "Closing sound system" );

	this->setUninitialized();
}

void WinSoundSystem::unprepareHeaders(  )
{
	MMRESULT res;
	//clear waveout headers
	for ( int x = 0; x < this->getNumBuffers( ); x++ ) {
		//clear wave headers
		res = waveOutUnprepareHeader( phwo, &waveHdr[ x ], sizeof( waveHdr[ x ] ) );
	}
}
void WinSoundSystem::assignNesSoundBuffer( NesApu::NesSoundBuffer* b ) {
	nesSoundBuffer_w = b;
}
