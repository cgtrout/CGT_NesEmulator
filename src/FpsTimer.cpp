#include "precompiled.h"

#include "FpsTimer.h"
using namespace FrontEnd;

/* 
==============================================
FPSTimer::FPSTimer()
==============================================
*/
FPSTimer::FPSTimer() {
	totalElapsedTime = 0.0f;
	frameNumber = 0.0f;
	currTime = 0;
	elapsedTime = 0;
	totalElapsedTime = 0;
	counterFrame = 0;

	timer = Timer::getInstance();
	
	
}

/* 
==============================================
double FPSTimer::getAvgFps()
==============================================
*/
double  FPSTimer::getAvgFps() {
	return frameNumber / totalElapsedTime;
}

/* 
==============================================
void FPSTimer::updateTimer(double elapsedTime)
==============================================
*/

void FPSTimer::updateTimer( double elapsedTime ) {
	
	//for fps calc
	frameNumber += 1;
	totalElapsedTime += elapsedTime;   
	
	//don't calculate averaged fps if we don't have enough
	//frames yet
	if( frameNumber < FRAMES_PER_SAMPLE ) {
		return;
	}
	
	//shuffle fps history by 1 on each element( and put new fps 
	//for this frame in the top element of the array
	frameFPS[ FRAMES_PER_SAMPLE-1 ] = 1/elapsedTime;
	for( int x = FRAMES_PER_SAMPLE-2; x >= 0; x-- ) {
		frameFPS[ x ] = frameFPS[ x+1 ];
	}

	//calculate average fps among all of the frames
	double total = 0;	
	for( int x = 0; x < FRAMES_PER_SAMPLE; x++ ) {
		total += frameFPS[ x ];
	}
	averagedFPS = total / FRAMES_PER_SAMPLE;
	
	//send value to gui counter - only send every
	//"COUNTER_SEND_RATE" frames
	if( counterFrame == COUNTER_SEND_RATE ) {
		SystemMain::getInstance()->frameCounter.setFps( averagedFPS );
		counterFrame = 0;
		return;
	}
	counterFrame++;
}

/* 
==============================================
void FPSTimer::clearTimer()
==============================================
*/
void FPSTimer::clearTimer() {
	currTime = 0; 
}

