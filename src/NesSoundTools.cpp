#include "Precompiled.h"

using namespace NesApu;

Divider::Divider():
		period(0), 
		counter(0){
}

LengthCounter::LengthCounter() : 
	counter(0), halted(false) {
}

EnvelopeGenerator::EnvelopeGenerator(): 
	counter(0), resetRequired(false), loop(false), disabled(true) {
}

Timer::Timer():
	halfTimer(false), currClock(0) {
}

SweepUnit::SweepUnit():
	enabled(false),
	negative(false),
	shiftAmt(0),
	squarePeriod(0),
	squareChannel(0),
	outstandingWrite(false),
	lastShiftVal(0){
}