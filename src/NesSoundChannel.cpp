#include "Precompiled.h"

using namespace NesApu;
SquareChannel::SquareChannel( ubyte c ) :
  duty(0), seqPos(0), volSetting(0) {
	setDacSize( 4 );
	setChannel( c );
	timer.setHalfTimer( true );
}

void SquareChannel::clock() {
	if( timer.clock() ) {
		if( ++seqPos == 8 ) {
			seqPos = 0;
		}
	}
}

void SquareChannel::clock240Hz( int clock ) {
	//clock envelope
	env.clock();
	
	if( clock == 1 || clock == 3 ) {
		//clock length counter
		lengthCounter.clock();
		
		//clock sweep unit
		sweep.setTimer( &this->timer );
		sweep.clock();
	} 
}

uword SquareChannel::getDacValue() {
	uword dac = 0;

	//handle envelope
	if( env.isDisabled() ) {
		dac = volSetting;
	} else {
		dac = env.getCounter();
	}

	//handle sweep
	if( timer.getPeriod() < 8 || sweep.getLastShiftVal() > 0x7ff ) {
		return 0;
	}

	_ASSERT( duty < 4 );
	_ASSERT( seqPos < 8 );

	//handle sequencer
	if( dutyTable[ duty ][ seqPos ] == 0) {
		return 0;
	}

	//handle length
	if( lengthCounter.getCounter() == 0 ) {
		return 0;
	}

	return dac;
}

void SquareChannel::regWrite0( ubyte mem ) {
	duty = ( mem & 0xc0 ) >> 6;
	//duty = 0;
	//duty = 100;
	
	ubyte envDisable = BIT( 4, mem );
	ubyte haltFlagSet = BIT( 5, mem );
	
	if( envDisable && haltFlagSet ) {
		lengthCounter.halt();
	} else if( !envDisable ) {
		env.setLoop( haltFlagSet );
	}
	volSetting = mem & 0x0f;
	//volSetting = 15;
	env.setPeriod( mem & 0x0f );
	
}

void SquareChannel::regWrite1( ubyte mem ) {
	sweep.setEnabled( BIT( 7, mem ) );
	sweep.setPeriod( ( mem & 0x70 ) >> 4 );
	sweep.setNegative( BIT( 4, mem ) );
	sweep.setShiftAmt( mem & 0x07 );
}

void SquareChannel::regWrite2( ubyte mem ) {
	setPeriodLow( mem );	
}

void SquareChannel::regWrite3( ubyte mem ) {
	setPeriodHigh( mem & 0x07 );
	lengthCounter.setLength( ( mem & 0xf8 ) >> 3 );

	//setPeriodHigh( mem );
	//setPeriodHigh( 0 );

}