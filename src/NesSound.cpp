#include "Precompiled.h"
#include "NesSound.h"
#include "imgui/imgui.h"
#include "implot/implot.h"
#include "pocketfft/pocketfft_hdronly.h"
#include "CgtGenericFunctions.h"

#include <cassert>
#include <limits>
#include <numeric>
#include <algorithm>

using namespace NesApu;

//number of clocks in 240hz
//precise value is 7,457.386363636422
static const int clocksPer240Hz = 7457;

//TODO set fracComp properly 
NesSoundBuffer::NesSoundBuffer( int bufLength ) :

	fracComp( 0, 77 ),
	buffer1( bufLength * 45 ),
	buffer2( bufLength * 45 ),
	sampleNum( 0 ),
	sampleTotal( 0 ),
	testBuffer( 2048 ),
	queuedAudioSizeBuffer( 50 ),
	remappedValuesHistory( 50 ),
	averageSampleIntervalBuffer( 1 ),
	highPassFilter440hz( 440, 44100 ),
	highPassFilter90hz( 90, 44100 ),
	lowPassFilter14khz( 14000, 44100 ),
	lowPassFilter20khz( 20000, 44100 ),
	lowPassFilter1mhzTo20khz( 22050, 1789773 )
{}

NesSoundBuffer::~NesSoundBuffer( ) {
}

//convert float value to 16bit value
Sint16 floatTo16Bit( float value ) {
	value = std::clamp( value, -1.0f, 1.0f );

	// convert float to integer in range [-32768, 32767]
	Sint16 result = (Sint16)( value * 32000 );
	return result;
}

auto lastTime = std::chrono::high_resolution_clock::now( );
float lastAverageQueuedBufferSize = 0;
bool lastWasShrink = false;
//cycle index - what cycle are we in
auto cycleIndex = 0u;
auto cycleTotal = 100u;
auto growPercent = 0.65f;
auto growPercentFudge = 0.0f;	//fudge up or down

double nes_clock_frequency = 1789773.2727272f;
double resampling_ratio = nes_clock_frequency / 44100;

auto soundStartTime = std::chrono::steady_clock::now( );
CgtLib::CircularBuffer<double> fpsHistory( 240 );

/*
==============================================
NesSoundBuffer::generateAudioBuffer
==============================================
*/
 std::vector<Sint16> NesSoundBuffer::generateAudioBuffer( Uint32 queuedAudioSize, float fps ) {
	auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now( ) - lastTime );
	lastTime = std::chrono::high_resolution_clock::now( );
	
	queuedAudioSizeBuffer.add( queuedAudioSize );
	//size_t desiredInterval = 41;

	//immediately set this so that emulation continues on other buffer
	size_t inputBufferPos = bufferPos;
	bufferPos = 0;
	auto selectedBuffer = activeBuffer;
	activeBuffer++;
	if( activeBuffer == 3 ) {
		activeBuffer = 1;
	}

	//create pointer to the active buffer
	std::vector<float>* bufferPtr = selectedBuffer == 1 ? &buffer1 : &buffer2;
	auto newResampleRatio = resampling_ratio;
	float averageQueuedAudioSize = CgtLib::calculateAverageFloat( queuedAudioSizeBuffer );

	if( averageQueuedAudioSize < 5000 ) {
		newResampleRatio = newResampleRatio * 0.994;
	}
	else {
		//adjust clock freq based on FPS
		nes_clock_frequency = 29760 * fps;
		newResampleRatio = nes_clock_frequency / 44100;
	}

	//_log->Write( "fps:%f   averageQueuedAudioSize: %f  newResampleRatio=%f", fps, averageQueuedAudioSize, newResampleRatio );

	

	remappedValuesHistory.add( newResampleRatio );

	size_t newBufferSize = static_cast<size_t>( static_cast<double>( inputBufferPos ) / ( newResampleRatio ) );
	std::vector<Sint16> newBuffer( newBufferSize );

	// Initialize the current input position
	double inputPosition = 0.0;

	// Loop through the output samples
	for( size_t i = 0; i < newBufferSize; i++ ) {
		// Calculate the integer part of the input position
		size_t index1 = static_cast<size_t>( inputPosition );

		// Calculate the next input index, ensuring it is within bounds
		size_t index2 = (std::min)( index1 + 1, bufferPtr->size( ) - 1 );

		// Calculate the fractional part of the input position
		double t = inputPosition - index1;
		//double t = 0;

		// Perform linear interpolation for the current output sample
		auto val1 = ( *bufferPtr )[ index1 ];
		auto val2 = ( *bufferPtr )[ index2 ];
		float output = static_cast<float>( ( 1.0 - t ) * val1 + t * val2 );
		//float output = val2;

		// Perform filtering
		output = highPassFilter90hz.process( output );
		output = highPassFilter440hz.process( output );
		output = lowPassFilter14khz.process( output );

		// Convert the output to a 16-bit value
		Sint16 convertedValue = floatTo16Bit( output );

		// Add the raw test data for ImPlot output
		testBuffer.add( output );

		// Store the converted value in the newBuffer
		newBuffer[ i ] = convertedValue;

		// Update the input position based on the resampling ratio
		inputPosition += newResampleRatio;
		//input_position += desiredInterval;
	}
	
	//clear old float buffer
	std::fill( bufferPtr->begin( ), bufferPtr->end( ), 0.0f );

	return newBuffer;
}

/*
==============================================
NesSoundBuffer::addSample
==============================================
*/
void NesSoundBuffer::addSample( float sample ) {
	//this is for the initial sample rate of 1.78Mhz
	//this is done to avoid aliasing
	sample = lowPassFilter1mhzTo20khz.process( sample );

	if( activeBuffer == 1 ) {
		buffer1[ bufferPos++ ] = sample;
	}
	else {
		buffer2[ bufferPos++ ] = sample;
	}
}

/*
==============================================
NesSoundBuffer::renderImGui
==============================================
*/
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
	
	if( ImPlot::BeginPlot( "remappedValuesHistory", ImVec2( 600, 250 ), ImPlotFlags_Crosshairs ) ) {
		SetupAxis( ImAxis_X1, "Time", ImPlotAxisFlags_NoLabel );
		//SetupAxis( ImAxis_Y1, "My Y-Axis" );
		SetupAxisLimits( ImAxis_Y1, 0, 50, 2 );
		PlotLine( "Buffer data", remappedValuesHistory.getBufferPtr( ), remappedValuesHistory.size( ) );
		EndPlot( );
	}

	if( ImPlot::BeginPlot( "queuedAudio", ImVec2( 600, 250 ), ImPlotFlags_Crosshairs ) ) {
		SetupAxis( ImAxis_X1, "Time", ImPlotAxisFlags_NoLabel );
		//SetupAxis( ImAxis_Y1, "My Y-Axis" );
		SetupAxisLimits( ImAxis_Y1, 0, 60000, 2 );
		PlotLine( "Buffer data", queuedAudioSizeBuffer.getBufferPtr( ), queuedAudioSizeBuffer.size( ) );
		EndPlot( );
	}


	if( ImPlot::BeginPlot( "NES Raw Sound Buffer Plot", ImVec2( 600, 250 ), ImPlotFlags_Crosshairs ) ) {
		SetupAxis( ImAxis_X1, "Time", ImPlotAxisFlags_NoLabel );
		SetupAxisLimits( ImAxis_Y1, -1, 1, 2 );
		PlotLine( "NES", testBuffer.getBufferPtr(), testBuffer.size());
		EndPlot( );
	}
	
	
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

/*
==============================================
NesSound Constructor
==============================================
*/
NesSound::NesSound( ) :
	cc(0),
	curr240Clock( 0 ),
	curr240ClockCycle(0),
	fracCC( 0, 3 ),
	frac240Clock( 0, 2 ),
	square0( 0 ),
	square1( 1 ),
	initialized( false ),
	queuedAudioSize( 0 ),
	buffer( AUDIO_SAMPLES * 2 ) //2 bytes per sample with enough room to hold two full buffers in the circular buffer
{
	//init fpsHistory to 60 to avoid averaging issues
	std::fill( fpsHistory.begin( ), fpsHistory.end( ), 60 );
}

/*
==============================================
NesSound::runTo
  desPpucc - desired clock to run to
==============================================
*/
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

/*
==============================================
NesSound::resetCC
==============================================
*/
void NesSound::resetCC() {
	cc = 0;
}

/*
==============================================
NesSound::makeSample
==============================================
*/
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

/*
==============================================
NesSound::queueSound
==============================================
*/
void NesSound::queueSound( ) {
	if( isInitialized( ) ) {
		//calculate FPS average - used by sound to fudge the sample rate
		//TODO need a better way to obtain this information rather than recalculating it here
		std::chrono::duration<double> elapsedTime = std::chrono::steady_clock::now( ) - soundStartTime;
		auto fps = (float)1 / elapsedTime.count();
		fpsHistory.add( fps );
		soundStartTime = std::chrono::steady_clock::now( );
		auto averageFPS = CgtLib::calculateAverageFloat( fpsHistory );

		//how much audio is queued up?
		queuedAudioSize = SDL_GetQueuedAudioSize( SDL_SoundDeviceId );

		//generate resampled audio and send to SDL
		auto newBuffer = buffer.generateAudioBuffer( queuedAudioSize, averageFPS );
		auto byteLength = sizeof( Sint16 ) * newBuffer.size();
		auto returnValue = SDL_QueueAudio( SDL_SoundDeviceId, newBuffer.data(), byteLength );
	}
}

/*
==============================================
NesSound::clock
==============================================
*/
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

/*
==============================================
NesSound::clock240Hz
==============================================
*/
void NesSound::clock240Hz( ) {
	//TODO handle 5 step

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

