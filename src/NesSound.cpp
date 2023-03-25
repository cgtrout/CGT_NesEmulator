#include "Precompiled.h"
#include "NesSound.h"
#include "imgui/imgui.h"
#include "implot/implot.h"
#include <cassert>
#include <limits>
#include <numeric>
#include <algorithm>

#include "pocketfft/pocketfft_hdronly.h"

using namespace NesApu;

//number of clocks in 240hz
//precise value is 7,457.386363636422
static const int clocksPer240Hz = 7457;

//TODO set fracComp properly 
NesSoundBuffer::NesSoundBuffer( int bufLength ):  
 
 fracComp( 0, 77 ),
 buffer1( bufLength*45 ),
 buffer2( bufLength*45 ),
 sampleNum(0),
 sampleTotal(0),
 testBuffer(2048),
 averageSampleIntervalBuffer(1),
 highPassFilter440hz(440, 44100),
 highPassFilter90hz(90, 44100),
 lowPassFilter14khz(14000, 44100),
 lowPassFilter20khz(20000, 44100 ),
 lowPassFilter1mhzTo20khz( 22050, 1789773)
{}

NesSoundBuffer::~NesSoundBuffer() {
}

//convert float value to 16bit value
Sint16 floatTo16Bit( float value ) {
	value = std::clamp( value, -1.0f, 1.0f );

	// convert float to integer in range [-32768, 32767]
	Sint16 result = (Sint16)( value * 32000 );
	return result;
}

auto lastTime = std::chrono::high_resolution_clock::now();

std::vector<Sint16> NesSoundBuffer::generateAudioBuffer( ) {
	auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now( ) - lastTime );
	lastTime = std::chrono::high_resolution_clock::now( );
	_log->Write( "Elapsed time=%lld milliseconds", elapsedTime.count( ) );

	
	//how many samples do we need - calculate from moving average
	/*
	float desiredIntervalFloat = bufferPos / (float)samples;
	averageSampleIntervalBuffer.add( desiredIntervalFloat );
	float total = std::accumulate( averageSampleIntervalBuffer.begin( ), averageSampleIntervalBuffer.end( ), 0 );
	size_t desiredInterval = 0;
	if( averageSampleIntervalBuffer.size( ) > 0 ) {
		desiredInterval = total / averageSampleIntervalBuffer.size( );
	}*/

	size_t desiredInterval = 40;
	
	size_t startBufferPos = bufferPos;

	bufferPos = 0;
	auto selectedBuffer = activeBuffer;
	
	//immediately set this so that emulation continues on other buffer
	activeBuffer++;

	if( activeBuffer == 3 ) {
		activeBuffer = 1;
	}
	
	//create pointer to the active buffer
	std::vector<float>* bufferPtr = selectedBuffer == 1 ? &buffer1 : &buffer2;

	//initialize export 16 bit buffer

	//newbuffer size half of float buffer
	//TODO should be calculated on actual data sizes
	size_t newBufferSize = startBufferPos / desiredInterval;
	std::vector<Sint16> newBuffer( newBufferSize );
	auto i = 0u, n = 0u;
	if( desiredInterval > 0 ) {
		
		for( ; i < bufferPtr->size( ) && n < newBufferSize; i += desiredInterval, n++ ) {
			float output = ( *bufferPtr )[ i ];

			//was a whole number generated?
			//int whole = fracComp.add( 45 );
			//sampleNum += whole;

			//random noise - remove comment to test filters with noise
			//output = static_cast<float>( rand( ) ) / RAND_MAX;

			//now do filtering
			//highpass filter 90hz
			output = highPassFilter90hz.process( output );

			//highpass filter 440hz
			output = highPassFilter440hz.process( output );

			//low pass 14000 hz
			output = lowPassFilter14khz.process( output );

			//now convert to 16 bit value
			Sint16 convertedValue = floatTo16Bit( output );

			//add raw test data for implot output
			testBuffer.add( output );

			newBuffer[ n ] = convertedValue;
		}
	}

	//_log->Write( "bufferPos:%d / samples:%d =  Desired interval:%d   Actual done=%d", startBufferPos, samples, desiredInterval, n );
	
	//clear old float buffer
	std::fill( bufferPtr->begin( ), bufferPtr->end( ), 0.0f );

	return newBuffer;
}

void NesSoundBuffer::addSample( float sample ) {
	//this is for the initial sample rate of 1.78Mhz
	//this is done to avoid aliasing
	//sample = lowPassFilter1mhzTo20khz.process( sample );

	if( activeBuffer == 1 ) {
		buffer1[ bufferPos++ ] = sample;
	}
	else {
		buffer2[ bufferPos++ ] = sample;
	}
}

void NesSoundBuffer::renderImGui( ) {
	// Create a custom ImGui window for the circular buffer visualization
	ImGui::Begin( "Audio Buffer Visualization" );

	using namespace ImPlot;

	/*
	if( ImPlot::BeginPlot( "Buffer1", ImVec2( 600, 250 ), ImPlotFlags_Crosshairs ) ) {
		SetupAxis( ImAxis_X1, "Time", ImPlotAxisFlags_NoLabel );
		SetupAxis( ImAxis_Y1, "My Y-Axis" );
		SetupAxisLimits( ImAxis_Y1, -30000, 30000, 2 );
		PlotLine( "Buffer data", buffer1.data( ), buffer1.size( ) );
		EndPlot( );
	}

	if( ImPlot::BeginPlot( "Buffer 2", ImVec2( 600, 250 ), ImPlotFlags_Crosshairs ) ) {
		SetupAxis( ImAxis_X1, "Time", ImPlotAxisFlags_NoLabel );
		SetupAxis( ImAxis_Y1, "My Y-Axis" );
		SetupAxisLimits( ImAxis_Y1, -30000, 30000, 2 );
		PlotLine( "Buffer data", buffer2.data( ), buffer2.size( ) );
		EndPlot( );
	}
	*/
	/*
	if( ImPlot::BeginPlot( "NES Raw Buffer Plot", ImVec2( -1, -1 ), ImPlotFlags_Crosshairs ) ) {
		SetupAxis( ImAxis_X1, "Time", ImPlotAxisFlags_NoLabel );
		SetupAxis( ImAxis_Y1, "My Y-Axis" );
		PlotLine( "NES", testBuffer.getBufferPtr(), testBuffer.size());
		EndPlot( );
	}*/


	if( ImPlot::BeginPlot( "DFT Graph", ImVec2( 600, 250 ), ImPlotFlags_Crosshairs ) ) {
		//prepare dft data
		auto n = testBuffer.size( );
		std::vector<std::complex<float>> fftBuffer( n / 2 + 1 );
		pocketfft::shape_t shape = { testBuffer.size( ) };
		pocketfft::stride_t stride_in = { sizeof( float ) };
		pocketfft::stride_t stride_out = { 2 * sizeof( float ) };
		size_t axis = 0;
		bool forward = true; // Forward FFT
		float fct = 1.0f; // Scaling factor
		size_t nthreads = 4; // Number of threads to use

		//do the fft
		pocketfft::r2c<float>( shape, stride_in, stride_out, axis, forward, testBuffer.getBufferPtr( ), fftBuffer.data( ), fct, nthreads );

		//calculate magnitudes
		std::vector<float> magnitudes( n / 2 + 1 );
		for( size_t i = 0; i < n / 2 + 1; ++i ) {
			magnitudes[ i ] = std::abs( fftBuffer[ i ] );
		}

		//calculate x axis freq data
		std::vector<float> frequencies( n / 2 + 1 );
		for( size_t i = 0; i < n / 2 + 1; ++i ) {
			frequencies[ i ] = ( i * 44100 ) / static_cast<float>( n );
		}

		SetupAxis( ImAxis_X1, "Time", ImPlotAxisFlags_NoLabel );
		SetupAxis( ImAxis_Y1, "My Y-Axis" );
		SetupAxisLimits( ImAxis_Y1, 0, 40, 2 );
		SetupAxisScale( ImAxis_X1, ImPlotScale_Log10 );
		PlotLine( "DFT", frequencies.data( ), magnitudes.data( ), magnitudes.size( ) );
		EndPlot( );
	}

	// End the ImGui window
	ImGui::End( );
}

NesSound::NesSound( ) :
	cc(0),
	curr240Clock( 0 ),
	curr240ClockCycle(0),
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

	//_log->Write( "nesSound::runTo() descc=%d bufferPos @ end = %d", descc, buffer.getBufferPos() );
}

void NesSound::resetCC() {
	cc = 0;
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

	buffer.addSample( output );
}

void NesSound::queueSound( ) {
	if( isInitialized( ) ) {
		auto newBuffer = buffer.generateAudioBuffer( );
		auto byteLength = sizeof( Sint16 ) * newBuffer.size();
		auto returnValue = SDL_QueueAudio( SDL_SoundDeviceId, newBuffer.data(), byteLength );
	}
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

