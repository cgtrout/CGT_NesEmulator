#include "Precompiled.h"


using namespace Sound;

/*
================================================================
================================================================
Class SoundBuffer - holds raw bufferdate
================================================================
================================================================
*/
SoundBuffer::SoundBuffer( ):
	buffer(0),
	bufferSize(0) {
}

SoundBuffer::~SoundBuffer() {
	deleteBuffer();
}

void SoundBuffer::clear() {
	for( int i = 0; i < bufferSize; ++i ) {
		buffer[i] = 0;
	}
}

void SoundBuffer::allocate( int size ) {
	if( buffer != 0 ) {
		deleteBuffer();
	}
	
	buffer = new word[ size ];
}

void SoundBuffer::deleteBuffer() {
	delete[] buffer;
}

/*
================================================================
================================================================
Class SoundSystem
  
  abstaacts generic nonsystem specific soundsystem
================================================================
================================================================
*/

SoundSystem::SoundSystem(  ):
 sampleRate( 44100 ),
 numBuffers( 2 ),
 bufferSize( 44100 / 2 ),
 initialized( false )
 {	 
	 
	 createBuffers();
	 clearBuffers();
};


//zero out the buffers
void SoundSystem::clearBuffers() {
	//ensure buffers is not null
	_ASSERTE( buffers != 0 );

	//clear buffers
	for( int b = 0; b < numBuffers; ++b ) {
		buffers[b].clear();
	}
}

void SoundSystem::deleteBuffers() {
	//ensure buffers is not null
	_ASSERTE( buffers != 0 );

	delete[] buffers;
}

SoundSystem::~SoundSystem() {
	deleteBuffers();
}

void SoundSystem::createBuffers() {
	//TODO check for success
	buffers = new SoundBuffer[ numBuffers ];
	for( int i = 0; i < numBuffers; i++ ) {		
		buffers[i].allocate( bufferSize );
	}
}
