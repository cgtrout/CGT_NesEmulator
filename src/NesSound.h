#if !defined( NesSound_INCLUDED )
#define NesSound_INCLUDED

#include "FractionAdder.h"
#include "NesSoundChannel.h"
#include "CgtDataStructures.h"

#include "CgtFilter.h"

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
		void fillExternalBuffer( Sint16* rawPointer, int size );

		//generate imgui visualization of sound buffers
		void renderImGui();

		//adds a sample to buffer - note: will average out a number 
		//of samples before adding to buffer
		void addSample( float sample );

		int getBufferPos() { return bufferPos; }
		void resetBufferPos() { bufferPos = 0; }

		//std::vector<Sint16>* getBuffer( ) { return &buffer; }

		//test buffer for viewing raw sample data (imgui)
		CgtLib::CircularBuffer<float> testBuffer;

		//filters
		CgtLib::HighPassFilter highPassFilter90hz;
		CgtLib::HighPassFilter highPassFilter440hz;
		CgtLib::ButterworthLowPassFilter lowPassFilter14khz;
		CgtLib::ButterworthLowPassFilter lowPassFilter20khz;

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

		//actual sound buffer
		std::vector<Sint16> buffer1;
		std::vector<Sint16> buffer2;
		int activeBuffer = 1;
		int bufferPos = 0;

		CgtLib::CircularBuffer<float> movingAverage;	
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
		  
		void setInitialized( ) { initialized = true; }
		void setUninitialized( ) { initialized = false; }
		bool isInitialized( ) { return initialized; }

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
				
		bool initialized;
		
		//sound buffer - holds sound data
		NesSoundBuffer buffer;

		//1.79Mhz clock
		void clock();

		//240 Hz clock 
		void clock240Hz();

		//current cycle
		CpuClockCycles cc;
		FractionAdder<PpuClockCycles> fracCC;

		//current cpu clock we are at relative to the next 240hz timing point
		int curr240Clock;
		
		//current stage in 4 step (mode 0)
		int curr240ClockCycle;
		FractionAdder<int> frac240Clock;		
	};


	inline PpuClockCycles NesSound::getCC() {
		return CpuToPpu( cc );
	}

	
	
};
#endif //NesSound_INCLUDED
