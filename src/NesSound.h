#if !defined( NesSound_INCLUDED )
#define NesSound_INCLUDED

#include "FractionAdder.h"
#include "NesSoundChannel.h"

namespace NesApu {
	/*
	================================================================
	================================================================
	Class NesSoundBuffer

	- holds raw sound data that will be output to soundcard
	- also responsible for filling external buffers with sound data

	  TODO what size should be used for each sample byte, word, or int?

	================================================================
	================================================================
	*/
	class NesSoundBuffer {
	  public:
		
		NesSoundBuffer( int bufLength );
		~NesSoundBuffer();
		
		//fills an external buffer with "size" number of bytes from 
		//internal buffer
		void fillExternalBuffer( word *rawPointer, int size );

		//adds a sample to buffer - note: will average out a number 
		//of samples before adding to buffer
		void addSample( word sample );

		int getBufferPos() { return bufferPos; }
		void resetBufferPos() { bufferPos = 0; }

	  private:
		//these are used for calculating average sample value
		int sampleNum;
		
		//fractional component of samplenum
		FractionAdder<int> fracComp;
		
		//total value of all subsamples for current sample
		int sampleTotal;

		//length of each sample - once this many samples have been
		//added they will be averaged and put into buffer
		int desiredSampleLength;

		//current pos in circular buffer
		int bufferPos;
		int bufferLength;
		int playPos;

		//actual sound buffer
		std::vector<uword> buffer;
	};

	/*
	================================================================
	================================================================
	Class NesSound

	- main class responsible for coordinating all of the nes 
	  sound generation classes
	================================================================
	================================================================
	*/	
	class NesSound {
	  public:
		NesSound();
		  
		//"catch up" the sound system - cc based on
		//offset from last vint
		void runTo( PpuClockCycles );
		
		//create a mixed sample
		void makeSample();

		//generate and return the status byte
		ubyte getStatusByte();

		PpuClockCycles getCC();

		void resetCC();

		NesSoundBuffer *getNesSoundBuffer() { return &buffer; }

		Image *getGraph();

		//nes channels
		SquareChannel	square0;
		SquareChannel	square1;
		TriangleChannel triangle;
		NoiseChannel	noise;
		DmcChannel		dmc;

	  private:
				
		//sound buffer - holds sound data
		NesSoundBuffer buffer;

		//1.79Mhz clock
		void clock();

		//240 Hz clock 
		void clock240Hz();

		//current cycle
		CpuClockCycles cc;
		FractionAdder<PpuClockCycles> fracCC;

		int curr240Clock;
		FractionAdder<int> frac240Clock;		
	};


	inline PpuClockCycles NesSound::getCC() {
		return CpuToMaster( cc );
	}

	
	
};
#endif //NesSound_INCLUDED
