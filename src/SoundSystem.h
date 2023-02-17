#if !defined( SoundSystem_INCLUDED )
#define SoundSystem_INCLUDED

#include "NesSound.h"

namespace Sound {
	/*
	================================================================
	================================================================
	Class SoundBuffer - holds raw bufferdate
	================================================================
	================================================================
	*/
	class SoundBuffer {
	  public:
		SoundBuffer( );
		~SoundBuffer();

		void allocate( int );

		word *get() { return buffer; }
		void clear();
	  private:
		void deleteBuffer();

		word *buffer;
		int bufferSize;
	};
		
	/*
	================================================================
	================================================================
	Class SoundSystem
	  
	  abstaacts generic nonsystem specific soundsystem
	================================================================
	================================================================
	*/
	class SoundSystem {
	  public:
		
		SoundSystem( );
		~SoundSystem();

		//initialize the sound system
		virtual void initialize() = 0; 	

		//shut down the sound system
		virtual void shutDown() = 0;

		//start the sound system
		virtual void start() = 0;

		//zero out buffers
		void clearBuffers();

		//has the snd system been initialized?
		bool isInitialized() { return initialized; }

		virtual void assignNesSoundBuffer( NesApu::NesSoundBuffer* ) = 0;

	  protected:

		//create and allocate buffers
		void createBuffers();
		void deleteBuffers();

		int getSampleRate() { return sampleRate; }
		int getNumBuffers() { return numBuffers; }
		int getBufferSize() { return bufferSize; }
		
		//void setBufferSize( int s ) { bufferSize = s; }
		
		SoundBuffer *getBuffer( int b ) { return &buffers[b]; }

		void setInitialized() { initialized = true; }
		void setUninitialized() { initialized = false; }
		

	  private:
		int sampleRate;
		int numBuffers;
		int bufferSize;
		
		bool initialized;

		SoundBuffer *buffers;
		
		//pointer to nesSoundBuffer
		NesApu::NesSoundBuffer *nesSoundBuffer;
		
	};

	


	class SoundSystemException : public CgtException {
	  public:
		SoundSystemException( string header, string m, bool s = true ) {
			::CgtException( header, m, s );
		}			
	};
}; //namespace

#endif