#include "Precompiled.h"
#include "NesSound.h"
#include "imgui/imgui.h"
#include "implot/implot.h"
#include <cassert>
#include <limits>
#include <numeric>

#include "pocketfft/pocketfft_hdronly.h"

using namespace NesApu;

//number of clocks in 240hz
//precise value is 7,457.386363636422
static const int clocksPer240Hz = 7457;

//TODO set fracComp properly 
NesSoundBuffer::NesSoundBuffer( int bufLength ):  
 fracComp( 0, 77 ),
 buffer1( bufLength ),
 buffer2( bufLength ),
 testBuffer(2048),
 highPassFilter440hz(440, 44100),
 highPassFilter90hz(90, 44100),
 lowPassFilter14khz(14000, 44100),
 lowPassFilter20khz(20000, 44100 ),
 movingAverage( 20 )
{}

NesSoundBuffer::~NesSoundBuffer() {
}

//boost::mutex mutex;
//samples is number of samples, and not actual byte size  of copy
void NesSoundBuffer::fillExternalBuffer( Sint16* ptr, int samples ) {
	if( samples > buffer1.size( ) || samples > buffer2.size( ) ) {
		throw CgtException( "fillExternalBUffer", "samples too large", true );
	}
	
	//alternate between two buffers
	//copy to given array, and then clear from buffer 
	if( activeBuffer == 1 ) {
		std::copy( buffer1.begin( ), buffer1.begin( ) + samples, ptr );
		std::fill( buffer1.begin( ), buffer1.end(), 0 );
	}
	else {
		std::copy( buffer2.begin( ), buffer2.begin( ) + samples, ptr );
		std::fill( buffer2.begin( ), buffer2.end(), 0 );
	}

	bufferPos = 0;
	activeBuffer++;

	if( activeBuffer == 3 ) {
		activeBuffer = 1;
	}
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
		auto avg = (Sint16)std::round((float)total / (float)movingAverage.size());

		//if we are running too fast skip a sample to avoid issues
		//question is why is there more samples being produced than there should be?
		if( bufferPos >= buffer1.size( ) ) {
			return;
		}

		if( activeBuffer == 1 ) {
			buffer1[ bufferPos++ ] = avg;
		}
		else {
			buffer2[ bufferPos++ ] = avg;
		}		
	}
}

void NesSoundBuffer::renderImGui( ) {
	// Create a custom ImGui window for the circular buffer visualization
	ImGui::Begin( "Audio Buffer Visualization" );

	using namespace ImPlot;

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

	_log->Write( "nesSound::runTo() descc=%d bufferPos @ end = %d", descc, buffer.getBufferPos() );
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
	//Sint16 result = (Sint16)( (value - 0.5) * 2 * 32000 );
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

	//random noise - remove comment to test filters with noise
	//output = static_cast<float>( rand( ) ) / RAND_MAX;

	//low pass 20000
	output = buffer.lowPassFilter20khz.process( output );

	//highpass filter 90hz
	output = buffer.highPassFilter90hz.process(output);

	//highpass filter 440hz
	output = buffer.highPassFilter440hz.process(output);

	//lowpass filter 14hz
	output = buffer.lowPassFilter14khz.process(output);
	
	//uncomment to add raw test data for implot output
	buffer.testBuffer.add( output );

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

