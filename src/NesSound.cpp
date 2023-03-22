#include "Precompiled.h"
#include "NesSound.h"
#include "imgui/imgui.h"
#include "implot/implot.h"
#include <cassert>
#include <limits>
#include <numeric>

//#include <boost/thread/mutex.hpp>

using namespace NesApu;

//number of clocks in 240hz
//precise value is 7,457.386363636422
static const int clocksPer240Hz = 7457;


//TODO set fracComp properly 
NesSoundBuffer::NesSoundBuffer( int bufLength ):  
 bufferLength( bufLength),
 fracComp( 0, 77 ),
 buffer( bufferLength ),
 playPos( 0 ),
 testBuffer(44100),
 highPassFilter440hz(440, 44100),
 highPassFilter90hz(90, 44100),
 lowPassFilter14hz(100, 44100),
 movingAverage( 40 )
{}

NesSoundBuffer::~NesSoundBuffer() {
}

//boost::mutex mutex;
//samples is number of samples, and not actual byte size  of copy
void NesSoundBuffer::fillExternalBuffer( Sint16* ptr, int samples ) {
	assert( samples <= bufferLength );
	if( this == NULL ) { 
		return;
	}
	
	//needed??
	//boost::mutex::scoped_lock lock(mutex);

	//may need to do in two steps
	bool twoStep = ( playPos + samples ) >= bufferLength;
		
	/*
	_log->Write( "\n\nfilling external buffer" );
	_log->Write( "sample count request = %d", samples );
	_log->Write( "apu buffer length = %d ", bufferLength);
	_log->Write( "playPos at start = %d ", playPos );
	_log->Write( "bufferPos = %d", bufferPos  );
	*/

	try {
		if( !twoStep ) {
			//copy in one step
			memcpy( ptr, &buffer[ playPos ], samples * 2 );
			playPos += samples;
			//_log->Write( "one step copy" );
			//_log->Write( "playPos at end = %d ", playPos );
			
			return;
		} else {
			_log->Write( "two step copy" );

			//first chunck
			int chunk1Size = bufferLength - playPos;
			memcpy( ptr, &buffer[ playPos ], chunk1Size * 2 );

			//_log->Write( "copying chunk 1 - chunk1Size = %d", chunk1Size );
			//playPos = 0;
			
			//second chunk
			int chunk2Size = samples - chunk1Size;
			memcpy( &ptr[ chunk1Size ] , &buffer[0], chunk2Size * 2 );
			//_log->Write( "copying chunk 2 - chunk2Size = %d", chunk2Size );
			playPos = chunk2Size;
			
			//_log->Write( "playPos at end = %d ", playPos );			
		}

		assert( playPos >= 0 && playPos < bufferLength );
	}
	catch( std::exception ) {
		//TODO handle this better
		_log->Write( "Exception generated in NesSoundBuffer::fillExternalBuffer" );
	}
}

void NesSoundBuffer::renderImGui() {
    // Create a custom ImGui window for the circular buffer visualization
    ImGui::Begin("Audio Buffer Visualization");

	using namespace ImPlot;
	
	if( ImPlot::BeginPlot( "Audio buffer Plot", ImVec2( -1, -1 ), ImPlotFlags_Crosshairs ) ) { 
		SetupAxis( ImAxis_X1, "Time", ImPlotAxisFlags_NoLabel );           
		SetupAxis( ImAxis_Y1, "My Y-Axis" );
		SetupAxisLimits( ImAxis_Y1, -30000, 30000, 2 );
		PlotLine( "Buffer data", buffer.data(), bufferLength );
		EndPlot( );
	}

	/*
	if( ImPlot::BeginPlot( "NES Raw Buffer Plot", ImVec2( -1, -1 ), ImPlotFlags_Crosshairs ) ) {
		SetupAxis( ImAxis_X1, "Time", ImPlotAxisFlags_NoLabel );
		SetupAxis( ImAxis_Y1, "My Y-Axis" );
		PlotLine( "NES", testBuffer.getBufferPtr(), testBuffer.size());
		EndPlot( );
	}*/

    // Display additional information if needed
    ImGui::Text("Play position: %d", playPos);
    ImGui::Text("Buffer length: %d", bufferLength);

    // End the ImGui window
    ImGui::End();
}

void NesSoundBuffer::addSample( Sint16 sample ) {

	movingAverage.add( sample );
	//should calculate 40 as a constexpr for clarity
	if( ++sampleNum == 40 ) {
		sampleNum = 0;
		
		//was a whole number generated?
		int whole = fracComp.add( 45 );
		sampleNum += whole;
		
		//use average value of last number of samples to create sample value	
		auto total = std::accumulate( movingAverage.begin( ), movingAverage.end( ), 0 );
		buffer[ bufferPos++ ] = (Sint16)std::round((float)total / (float)movingAverage.size());
		if( bufferPos == bufferLength ) {
			bufferPos = 0;
		}
	}
}

NesSound::NesSound( ) :
	curr240Clock( 0 ),
	fracCC( 0, 3 ),
	frac240Clock( 0, 2 ),
	square0( 0 ),
	square1( 1 ),
	initialized( false ),
	
	buffer( AUDIO_SAMPLES * 2 ) //2 bytes per sample with enough room to hold two full buffers in the circular buffer
{
}

//descc desired clock cycle
void NesSound::runTo( PpuClockCycles desPpucc ) {
	CpuClockCycles descc = PpuToCpu( desPpucc );
	
	//descc must be greater than cc
	assert( descc > cc );

	//currc = descc;
	
	//if fraction adds up to whole numbers add it to current cc;
	cc += fracCC.add( PpuToCpuMod( desPpucc ) );

	for( ; cc < descc; ++cc ) {
		clock();
		makeSample();
	}

	//_log->Write( "nesSound::runTo() bufferPos @ end = %d", buffer.getBufferPos() );
}

void NesSound::resetCC() {
	cc = 0;
}

#include <cstdint> // for int16_t type


//convert float value to 16bit value
//assumes that float is a sample between 0.0 and 1.0
Sint16 floatTo16Bit( float value ) {
	// clamp value to [0.0, 1.0] range
	if ( value < 0.0f ) {
		value = 0.0f;
	} else if ( value > 1.0f ) {
		value = 1.0f;
	}

	// convert float to integer in range [-32768, 32767]
	Sint16 result = ( Sint16 )( value * 30000 );
	return result;
}

void NesSound::makeSample() {
	float squareOut = 0;
	if( square0.getDacValue() == 0 && square1.getDacValue() == 0 ) {
		squareOut = 0;
	} else {
		//TODO slow - use tables
		squareOut = (float)95.88 / ( ( 8128 / ( square0.getDacValue() + square1.getDacValue() ) ) + 100 );
		//_log->Write( "squareOut=%f", squareOut );
		//if( square1.getDacValue() == 0 ) squareOut = 0;
		//else squareOut = (float)95.88 / ( ( 8128 / ( square1.getDacValue() ) ) + 100 );
	}
	float output = squareOut;

	//highpass filter 90hz
	//output = buffer.highPassFilter90hz.process(output);

	//highpass filter 440hz
	//output = buffer.highPassFilter440hz.process(output);

	//lowpass filter 14hz
	//output = buffer.lowPassFilter14hz.process(output);
	
	//uncomment to add raw test data for implot output
	//buffer.testBuffer.add( output );

	//convert and scale to signed 16bit int
	Sint16 bitval = floatTo16Bit( output );

	//_log->Write( "float=%f  16bit=%d", output, bitval);
	buffer.addSample( bitval );
}

inline void NesSound::clock() {
	//increment clocks
	square0.clock();
	square1.clock();
	//triangle.clock();
	//noise.clock();
	//dmc.clock();
	
	//check for 240hz clock
	if( ++curr240Clock == clocksPer240Hz ) {
		clock240Hz();
		curr240Clock = 0;
		
		//add 1 to clock if one whole number has been produced
		curr240Clock += frac240Clock.add( 1 );	
	}
}

//TODO handle 5 step
void NesSound::clock240Hz( ) {
	square0.clock240Hz( curr240ClockCycle );
	square1.clock240Hz( curr240ClockCycle );
	//triangle.clock240Hz( curr240Clock );
	//noise.clock240Hz( curr240Clock );
	//dmc.clock240Hz( curr240Clock );
	
	//increment 240 clock cycle and reset it if necessary
	if( ++curr240ClockCycle == 4 ) {
		curr240ClockCycle = 0;
	}

	//TODO need to implement 4-step (mode 0) of 'frame sequencer'
}

//
//
//Memory registers
//
//

//square 0 channel

