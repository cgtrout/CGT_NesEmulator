#if !defined( NesSoundChannel_INCLUDED )
#define NesSoundChannel_INCLUDED

#include "NesSoundTools.h"

namespace NesApu {	
	/*
	================================================================
	================================================================
	Class NesChannel

	- base class for all nes channels
	================================================================
	================================================================
	*/
	class NesChannel {
	  public:
		NesChannel( ) {}

		//1.79Mhz clock
		ubyte clock();

		//240hz clock
		ubyte clock240Hz( int count );  //count must be between 0-4

		//get current value in DAC
		int getDacValue();

	  protected:
		
		//get bit-size of dac
		int getDacSize();
		void setDacSize( ubyte );

	  private:
		ubyte dacSize;
		uword dacValue;
	};

	inline void NesChannel::setDacSize( ubyte val ) {
		dacSize = val;
	}

	inline int NesChannel::getDacSize() {
		return dacSize;
	}

	inline int NesChannel::getDacValue() {
		return dacValue;
	}

	/*
	================================================================
	================================================================
	class SquareChannel
	================================================================
	================================================================
	*/
	class SquareChannel : public NesChannel {
	  public:
		SquareChannel( ubyte channel );
		void clock();
		void clock240Hz( int );

		uword getDacValue();

		void setChannel( ubyte c );

		void regWrite0( ubyte );
		void regWrite1( ubyte );
		void regWrite2( ubyte );
		void regWrite3( ubyte );

	  private:
		//duty of square wave must be 0-3
		ubyte duty;

		SweepUnit sweep;
		Timer timer;
		LengthCounter lengthCounter;
		ubyte seqPos;
		ubyte channel;

		ubyte volSetting;

		EnvelopeGenerator env;

		//force channel to be set
		SquareChannel() {}

		void setPeriodLow( ubyte );
		void setPeriodHigh( ubyte );
	};

	

	inline void SquareChannel::setChannel( ubyte c ) {
		_ASSERT( c == 0 || c == 1 );
		channel = c;
		sweep.setSquareChannel( c );
	}

	
	
	static ubyte dutyTable [4][8] = {
	  /* 0 */	{ 0, 1, 0, 0, 0, 0, 0, 0 },
	  /* 1 */	{ 0, 1, 1, 0, 0, 0, 0, 0 },
	  /* 2 */	{ 0, 1, 1, 1, 1, 0, 0, 0 },
	  /* 3 */	{ 1, 0, 0, 1, 1, 1, 1, 1 },
	};

	inline void SquareChannel::setPeriodLow( ubyte val ) {
		uword period = timer.getPeriod();
		period &= 0xFF00;
		
		period += val + 1;
		
		//subtract one to offset automatic addition
		timer.setPeriod( period );
	}
	
	inline void SquareChannel::setPeriodHigh( ubyte val ) {
		uword period = timer.getPeriod();
		period &= 0x00ff;
		period += val << 8;
		
		//now allow addition of 1
		timer.setPeriod( period );
	}

	class TriangleChannel : public NesChannel {

	};

	class NoiseChannel : public NesChannel {

	};

	class DmcChannel : public NesChannel {

	};
};//namespace

#endif