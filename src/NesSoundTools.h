#pragma once

#include <cassert>

namespace NesApu {
	/*
	================================================================
	================================================================
	Class Divider
	================================================================
	================================================================
	*/
	class Divider {
	  public:
		
		Divider();

		ubyte clock();

		void setPeriod( uword );
		uword getCounter();
		uword getPeriod();

		void reset();

	  private:
		uword period;
		uword counter;
	};


	__forceinline ubyte Divider::clock() {
		if( counter == 0 ) {
			counter = period;
			return 1;
		}
		else if( --counter == 0 ) {
			counter = period;
			return 1;
		}
		return 0;
	}

	__forceinline void Divider::setPeriod( uword p ) {
		period = p;
	}

	__forceinline uword Divider::getPeriod( ) { return period; }

	__forceinline uword Divider::getCounter() {
		return counter;
	}

	__forceinline void Divider::reset() {
		counter = period;
	}

	/*
	================================================================
	================================================================
	Class LengthCounter
	================================================================
	================================================================
	*/
	class LengthCounter {
	public:
		LengthCounter();
		void clock();
		void halt();

		//sets length of counter based on lookup table
		void setLength( int index );

		uword getCounter();
	private:
		uword counter;
		bool halted;
	};

	inline void LengthCounter::clock() {
		if( !halted  && counter != 0 ) {
			--counter;
		}

	}

	inline void LengthCounter::halt() {
		halted = true;
	}

	//used to load counter
	static ubyte lengthTable[0x10][0x02] = 	{	

	// bits 	    bit 3
	// 7-4		   0	  1
	//---------------------------
	  /* 0 */	{ 0x0a, 0xfe },
	  /* 1 */	{ 0x14, 0x02 },
	  /* 2 */	{ 0x28, 0x04 },
	  /* 3 */	{ 0x50, 0x06 },
	  /* 4 */	{ 0xa0, 0x08 },
	  /* 5 */	{ 0x3c, 0x0a },
	  /* 6 */	{ 0x0e, 0x0c },
	  /* 7 */	{ 0x1a, 0x0e },
	  /* 8 */	{ 0x0c, 0x10 },
	  /* 9 */	{ 0x18, 0x12 },
	  /* a */	{ 0x30, 0x14 },
	  /* b */	{ 0x60, 0x16 },
	  /* c */	{ 0xc0, 0x18 },
	  /* d */	{ 0x48, 0x1a },
	  /* e */	{ 0x10, 0x1c },
	  /* f */	{ 0x20, 0x1e },
	};

	inline void LengthCounter::setLength( int index ) {
		counter = lengthTable[ ( index & 0xf0 ) >> 4 ][ ( index & 0x08 ) >> 3 ];
	}

	inline uword LengthCounter::getCounter() {
		return counter;
	}

	/*
	================================================================
	================================================================
	Class EnvelopeGenerator
	================================================================
	================================================================
	*/
	class EnvelopeGenerator {
	  public:
		EnvelopeGenerator();
		void clock();

		uword getCounter();

		void setPeriod( ubyte period );

		void setLoop( ubyte );

		void setDisabled( ubyte );

		ubyte isDisabled();
	  private:
		Divider divider;
		uword counter;

		//set to true if a write was done since last clock
		ubyte resetRequired;
		ubyte loop;
		ubyte disabled;
	};

	//TODO check to see if there was write to fourth channel reg			
	inline void EnvelopeGenerator::clock( ) {
		ubyte dividerClocked = divider.clock();

		if( dividerClocked ) {
			if( loop && counter == 0 ) {
				counter = 15;
			} else if( counter != 0 ) {
				--counter;
			}
		}
	}

	inline uword EnvelopeGenerator::getCounter() {
		return counter;
	}

	inline void EnvelopeGenerator::setPeriod( ubyte p ) {
		divider.setPeriod( p + 1 );
	}

	inline void EnvelopeGenerator::setLoop( ubyte val ) {
		loop = val;
	}

	inline void EnvelopeGenerator::setDisabled( ubyte val ) {
		disabled = val;
	}

	inline ubyte EnvelopeGenerator::isDisabled() {
		return disabled;
	}

	/*
	================================================================
	================================================================
	Class Timer

	  TODO handle "timer / 2" for square channels
	================================================================
	================================================================
	*/
	class Timer {
	  public:
		Timer();
		
		ubyte clock();
		
		void setPeriod( uword p );
		uword getPeriod();

		//if set to true will be a timer / 2
		//(for use with square channel)
		void setHalfTimer( bool );

	  private:
		Divider divider;
		
		//divide by two on timer
		bool halfTimer;
		ubyte currClock;
	};

	

	__forceinline void Timer::setHalfTimer( bool val ) {
		halfTimer = val;
	}

	__forceinline ubyte Timer::clock() {
		if( halfTimer ) {
			if( currClock++ == 1 ) {
				ubyte dividerOut = divider.clock( );
				currClock = 0;
				return dividerOut;
			}
			else {
				return 0;
			}
		} else {
			return divider.clock();
		}
	}

	__forceinline uword Timer::getPeriod() {
		return divider.getPeriod();
	}

	__forceinline void Timer::setPeriod( uword p ) {
		divider.setPeriod( p );
	}

	/*
	================================================================
	================================================================
	Class SweepUnit
	================================================================
	================================================================
	*/
	class SweepUnit {
	  public:
		SweepUnit();

		//sweep changes timer period, so it needs access to the timer object
		void setTimer( Timer* t );
		
		//sets period for sweep units divider  
		void setPeriod( ubyte );
		
		//only clocked every 120hz
		void clock();

		//sets pointer to period of square channel
		void setSquarePeriodPtr( uword *squarePeriod );
		
		//must be 0 or 1
		void setSquareChannel( ubyte );

		void setEnabled( ubyte );
		void setNegative( ubyte );
		void setShiftAmt( ubyte );

		uword getLastShiftVal();
	  
	  private:
	
		Divider divider;
		Timer* timer;
		ubyte enabled;
		ubyte negative;
		ubyte shiftAmt;
		uword lastShiftVal;

		ubyte squareChannel;

		//was a write done to sweep since last clock?
		bool outstandingWrite;
	};

	inline void SweepUnit::setTimer( Timer* t ) {
		timer = t;
	}

	inline void SweepUnit::setPeriod( ubyte val ) {
		divider.setPeriod( val + 1 );
	}

	inline void SweepUnit::clock( ) {
		ubyte dividerClocked = divider.clock();
		if( outstandingWrite ) {
			divider.reset();
			outstandingWrite = false;
		}

		if( dividerClocked ) {
			uword period = timer->getPeriod( );

			uword shiftVal = period >> shiftAmt;
			if( negative ) {
				//invert
				shiftVal = ~shiftVal;
				if( squareChannel == 1 ) {
					++shiftVal;
				}
			}
			if( shiftVal <= 0x7ff && enabled && shiftAmt > 0 ) {
				period += shiftVal;
			}
			lastShiftVal = shiftVal;
			timer->setPeriod( period );
		}
	}

	inline void SweepUnit::setSquareChannel( ubyte ch ) {
		assert( ch == 0 || ch == 1 );
		squareChannel = ch;
	}

	inline void SweepUnit::setEnabled( ubyte val ) {
		enabled = val;
	}

	inline void SweepUnit::setNegative( ubyte val  ) {
		negative = val;
	}

	inline void SweepUnit::setShiftAmt( ubyte val ) {
		shiftAmt = val;
	}

	inline uword SweepUnit::getLastShiftVal() {
		return lastShiftVal;
	}
};
